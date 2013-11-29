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
#include <string.h>
#include <libsoup/soup.h>
#include "KmsHttpPost.h"

#define OBJECT_NAME "HttpPost"
#define MIME_MULTIPART_FORM_DATA "multipart/form-data"

#define GST_CAT_DEFAULT kms_http_post_debug_category
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define KMS_HTTP_POST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_HTTP_POST, KmsHttpPostPrivate))

typedef enum {
  MULTIPART_FIND_BOUNDARY,
  MULTIPART_READ_HEADERS,
  MULTIPART_READ_CONTENT
} ParseState;

typedef struct _KmsHttpPostMultipart {
  gchar *boundary;
  ParseState state;
  gchar *tmp_buff;
  guint len;
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
kms_http_post_concat_previous_buffer (KmsHttpPost *self, const char **start,
                                      const char **end)
{
  if (self->priv->multipart->tmp_buff == NULL)
    return;

  if (self->priv->multipart->tmp_buff <= *start &&
      *start <= self->priv->multipart->tmp_buff + self->priv->multipart->len) {
    /* Memory overlap => Do not reallocate more memory in this case */
    return;
  }

  self->priv->multipart->tmp_buff = (char *) g_realloc (
                                      self->priv->multipart->tmp_buff,
                                      self->priv->multipart->len + (*end - *start) );
  memmove (self->priv->multipart->tmp_buff + self->priv->multipart->len,
           *start, *end - *start);
  self->priv->multipart->len += (*end - *start);

  *start = self->priv->multipart->tmp_buff;
  *end = self->priv->multipart->tmp_buff + self->priv->multipart->len;
}

static void
kms_http_post_find_boundary (KmsHttpPost *self, const char **start,
                             const char **end, const char *boundary, int boundary_len)
{
  const char *b;

  if (self->priv->multipart->tmp_buff != NULL)
    kms_http_post_concat_previous_buffer (self, start, end);

  for (b = (const char *) memchr (*start, '-', *end - *start); b != NULL;
       b = (const char *) memchr (b + 2, '-', *end - (b + 2) ) ) {

    if (b + boundary_len + 4 > *end) {
      /* boundary does not fit in this buffer */
      gchar *mem = NULL;

      if (self->priv->multipart->tmp_buff != NULL)
        mem = self->priv->multipart->tmp_buff;

      self->priv->multipart->tmp_buff = (gchar *) g_memdup (b, *end - b);
      self->priv->multipart->len = *end - b;

      if (mem != NULL)
        g_free (mem);

      return;
    }

    /* Check for "--boundary" */
    if (b[1] != '-' ||
        memcmp (b + 2, boundary, boundary_len) != 0)
      continue;

    /* Check for "--" or "\r\n" after boundary */
    if ( (b[boundary_len + 2] == '-' && b[boundary_len + 3] == '-') ||
         (b[boundary_len + 2] == '\r' && b[boundary_len + 3] == '\n') ) {
      self->priv->multipart->state = MULTIPART_READ_HEADERS;

      if (*end <= b + boundary_len + 4) {
        /* Free temporal buffer */
        break;
      }

      *start = b + boundary_len + 4;
      return;
    }
  }

  if (self->priv->multipart->tmp_buff != NULL) {
    g_free (self->priv->multipart->tmp_buff);
    self->priv->multipart->tmp_buff = NULL;
  }
}

static void
kms_http_post_parse_header (KmsHttpPost *self, const char *start,
                            const char *end)
{
  gchar *header;

  header = strndup (start, end - start);
  GST_DEBUG ("TODO Parse header: *%s*", header);
  g_free (header);
}

static void
kms_http_post_read_headers (KmsHttpPost *self, const char **start,
                            const char **end)
{
  const char *b;

  if (self->priv->multipart->tmp_buff != NULL)
    kms_http_post_concat_previous_buffer (self, start, end);

  b = *start;

  while (b <= *end) {
    const char *newline = (const char *) memchr (b, '\n', *end - b);

    if (newline == NULL) {
      /* header does not fit in this buffer */
      gchar *mem = NULL;

      if (self->priv->multipart->tmp_buff != NULL)
        mem = self->priv->multipart->tmp_buff;

      self->priv->multipart->tmp_buff = (gchar *) g_memdup (b, *end - b);
      self->priv->multipart->len = *end - b;

      if (mem != NULL)
        g_free (mem);

      return;
    }

    /* Check if this is a blank line */
    if (newline - b == 1 && b[0] == '\r' && b[1] == '\n') {
      self->priv->multipart->state = MULTIPART_READ_CONTENT;

      if (*end <= newline + 1) {
        /* Free temporal buffer */
        break;
      }

      *start = newline + 1;
      return;
    }

    kms_http_post_parse_header (self, b, newline);

    /* Search next header starting from the next character following '\n' */
    b = newline + 1;
  }

  if (self->priv->multipart->tmp_buff != NULL) {
    g_free (self->priv->multipart->tmp_buff);
    self->priv->multipart->tmp_buff = NULL;
  }
}

static void
kms_http_post_parse_multipart_data (KmsHttpPost *self, const char *data,
                                    const char *end)
{
  switch (self->priv->multipart->state) {
  case MULTIPART_FIND_BOUNDARY:
    /* skip preamble */
    kms_http_post_find_boundary (self, &data, &end,
                                 self->priv->multipart->boundary,
                                 strlen (self->priv->multipart->boundary) );

    if (self->priv->multipart->state == MULTIPART_FIND_BOUNDARY)
      break;

  case MULTIPART_READ_HEADERS:
    kms_http_post_read_headers (self, &data, &end);

    if (self->priv->multipart->state == MULTIPART_READ_HEADERS)
      break;

  case MULTIPART_READ_CONTENT:
    GST_DEBUG ("TODO: Read content");
  }
}

static void
got_chunk_cb (SoupMessage *msg, SoupBuffer *chunk, gpointer data)
{
  KmsHttpPost *self = KMS_HTTP_POST (data);

  if (self->priv->multipart == NULL)
    GST_DEBUG ("Process raw data");
  else
    kms_http_post_parse_multipart_data (self, chunk->data,
                                        chunk->data + chunk->length);
}

static void
kms_http_post_destroy_multipart (KmsHttpPost *self)
{
  if (self->priv->multipart == NULL)
    return;

  if (self->priv->multipart->boundary != NULL)
    g_free (self->priv->multipart->boundary);

  if (self->priv->multipart->tmp_buff != NULL)
    g_free (self->priv->multipart->tmp_buff);

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

  if (strncmp (content_type, "multipart/", 10) == 0) {
    if (strlen (content_type) >= 19 &&
        strncmp (content_type + 11, "form-data", 9) ) {
      /* Content-Type: multipart/form-data */
      kms_http_post_init_multipart (self);
      self->priv->multipart->boundary =
        g_strdup ( (gchar *) g_hash_table_lookup (params, "boundary") );

      if (self->priv->multipart->boundary == NULL) {
        GST_WARNING ("Malformed multipart POST request");
        kms_http_post_destroy_multipart (self);
        soup_message_set_status (self->priv->msg, SOUP_STATUS_NOT_ACCEPTABLE);
        goto end;
      }
    } else {
      GST_WARNING ("Unsupported multipart format: %s", content_type);
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