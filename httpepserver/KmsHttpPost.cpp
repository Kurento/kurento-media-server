/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */
#include <libsoup/soup.h>
#include "KmsHttpPost.h"

#define OBJECT_NAME "HttpPost"
#define MIME_MULTIPART_FORM_DATA "multipart/form-data"

#define GST_CAT_DEFAULT kms_http_post_debug_category
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define KMS_HTTP_POST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_HTTP_POST, KmsHttpPostPrivate))
typedef struct _KmsHttpPostMultipart {
  gchar *boundary;
  gboolean got_boundary;
  gboolean got_last_boundary;
} KmsHttpPostMultipart;

struct _KmsHttpPostPrivate {
  KmsHttpPostMultipart *multipart;
  SoupMessage *msg;
  gulong handler_id;
};

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (KmsHttpPost, kms_http_post,
                         G_TYPE_OBJECT,
                         GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, OBJECT_NAME,
                             0, "debug category for " OBJECT_NAME " element") )
/* properties */
enum {
  PROP_0,

  PROP_MESSAGE,
  N_PROPERTIES
};

#define KMS_HTTP_PORT_DEFAULT_MESSAGE NULL

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* signals */
enum {
  GOT_DATA,
  LAST_SIGNAL
};

static guint obj_signals[LAST_SIGNAL] = { 0 };

static void
got_chunk_cb (SoupMessage *msg, SoupBuffer *chunk, gpointer data)
{
  GST_DEBUG ("TODO: Process data");
}

static void
kms_http_post_destroy_multipart (KmsHttpPost *self)
{
  if (self->priv->multipart == NULL)
    return;

  if (self->priv->multipart->boundary != NULL)
    g_free (self->priv->multipart->boundary);

  g_slice_free (KmsHttpPostMultipart, self->priv->multipart);
  self->priv->multipart = NULL;
}

static void
kms_http_post_init_multipart (KmsHttpPost *self)
{
  if (self->priv->multipart != NULL) {
    GST_WARNING ("Multipart data is already initialized");
    kms_http_post_destroy_multipart (self);
  }

  self->priv->multipart = g_slice_new0 (KmsHttpPostMultipart);
}

static void
kms_http_post_configure_msg (KmsHttpPost *self)
{
  const gchar *content_type;
  GHashTable *params = NULL;

  content_type =
    soup_message_headers_get_content_type (self->priv->msg->request_headers,
        &params);

  if (content_type == NULL) {
    GST_WARNING ("Content-type header is not present in request");
    soup_message_set_status (self->priv->msg, SOUP_STATUS_NOT_ACCEPTABLE);
    goto end;
  }

  if (g_strcmp0 (content_type, MIME_MULTIPART_FORM_DATA) == 0) {
    kms_http_post_init_multipart (self);
    self->priv->multipart->boundary = g_strdup ( (gchar *) g_hash_table_lookup (params,
                                      "boundary") );

    if (self->priv->multipart->boundary == NULL) {
      GST_WARNING ("Malformed multipart POST request");
      kms_http_post_destroy_multipart (self);
      soup_message_set_status (self->priv->msg, SOUP_STATUS_NOT_ACCEPTABLE);
      goto end;
    }
  }

  soup_message_set_status (self->priv->msg, SOUP_STATUS_OK);

  /* Get chunks without filling-in body's data field after */
  /* the body is fully sent/received */
  soup_message_body_set_accumulate (self->priv->msg->request_body, FALSE);

  self->priv->handler_id = g_signal_connect (self->priv->msg, "got-chunk",
                           G_CALLBACK (got_chunk_cb), self);
end:

  if (params != NULL)
    g_hash_table_destroy (params);
}

static void
kms_http_post_release_message (KmsHttpPost *self)
{
  if (self->priv->msg == NULL)
    return;

  if (self->priv->handler_id != 0L) {
    g_signal_handler_disconnect (self->priv->msg, self->priv->handler_id);
    self->priv->handler_id = 0L;
  }

  g_object_unref (self->priv->msg);
  self->priv->msg = NULL;
}

static void
kms_http_post_set_property (GObject *obj, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  switch (prop_id) {
  case PROP_MESSAGE:
    kms_http_post_release_message (self);
    kms_http_post_destroy_multipart (self);
    self->priv->msg = SOUP_MESSAGE (g_object_ref (g_value_get_object (value) ) );
    kms_http_post_configure_msg (self);
    break;

  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    break;
  }
}

static void
kms_http_post_get_property (GObject *obj, guint prop_id, GValue *value,
                            GParamSpec *pspec)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  switch (prop_id) {
  case PROP_MESSAGE:
    g_value_set_object (value, self->priv->msg);
    break;

  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    break;
  }
}

static void
kms_http_post_dispose (GObject *obj)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  kms_http_post_release_message (self);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_post_parent_class)->dispose (obj);
}

static void
kms_http_post_finalize (GObject *obj)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  kms_http_post_destroy_multipart (self);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_post_parent_class)->finalize (obj);
}

static void
kms_http_post_class_init (KmsHttpPostClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = kms_http_post_set_property;
  gobject_class->get_property = kms_http_post_get_property;
  gobject_class->dispose = kms_http_post_dispose;
  gobject_class->finalize = kms_http_post_finalize;

  obj_properties[PROP_MESSAGE] =
    g_param_spec_object ("soup-message",
                         "Soup message object",
                         "Message to get data from",
                         SOUP_TYPE_MESSAGE,
                         (GParamFlags) (G_PARAM_READWRITE) );

  g_object_class_install_properties (gobject_class,
                                     N_PROPERTIES,
                                     obj_properties);

  obj_signals[GOT_DATA] =
    g_signal_new ("got-data",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (KmsHttpPostClass, got_data), NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  /* Registers a private structure for an instantiatable type */
  g_type_class_add_private (klass, sizeof (KmsHttpPostPrivate) );
}

static void
kms_http_post_init (KmsHttpPost *self)
{
  self->priv = KMS_HTTP_POST_GET_PRIVATE (self);
  self->priv->handler_id = 0L;
}