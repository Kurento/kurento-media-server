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

#define GST_CAT_DEFAULT kms_http_post_debug_category
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define KMS_HTTP_POST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_HTTP_POST, KmsHttpPostPrivate))
struct _KmsHttpPostPrivate {
  SoupMessage *msg;
  guint id;
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
kms_http_post_set_property (GObject *obj, guint prop_id,
                            const GValue *value, GParamSpec *pspec)
{
  KmsHttpPost *self = KMS_HTTP_POST (obj);

  switch (prop_id) {
  case PROP_MESSAGE:
    if (self->priv->msg != NULL)
      g_object_unref (self->priv->msg);

    /* TODO: Connect to "got-chunk" signal */
    self->priv->msg = SOUP_MESSAGE (g_object_ref (g_value_get_object (value) ) );
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

  if (self->priv->msg != NULL) {
    g_object_unref ( G_OBJECT (self->priv->msg) );
    self->priv->msg = NULL;
  }

  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_post_parent_class)->dispose (obj);
}

static void
kms_http_post_finalize (GObject *obj)
{
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
}