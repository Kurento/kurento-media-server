/*
 * Copyright (C) 2013 Kurento
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gst/gst.h>
#include "KmsHttpEPServer.h"

#define OBJECT_NAME "HttpEPServer"

#define GST_CAT_DEFAULT kms_http_ep_server_debug_category
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define KMS_HTTP_EP_SERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_HTTP_EP_SERVER, KmsHttpEPServerPrivate))
struct _KmsHttpEPServerPrivate {
  guint count;
};

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (KmsHttpEPServer, kms_http_ep_server,
    G_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, OBJECT_NAME,
        0, "debug category for " OBJECT_NAME " element") )

static void
kms_http_ep_server_start_impl (KmsHttpEPServer *self)
{
  GST_DEBUG ("Start: Do something");
}

static void
kms_http_ep_server_stop_impl (KmsHttpEPServer *self)
{
  GST_DEBUG ("Stop: Do something");
}

static void
kms_http_ep_server_dispose (GObject *obj)
{
  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_ep_server_parent_class)->dispose (obj);
}

static void
kms_http_ep_server_finalize (GObject *obj)
{
  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_ep_server_parent_class)->finalize (obj);
}

static void
kms_http_ep_server_class_init (KmsHttpEPServerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = kms_http_ep_server_dispose;
  gobject_class->finalize = kms_http_ep_server_finalize;

  /* Set public virtual methods */
  klass->start = kms_http_ep_server_start_impl;
  klass->stop = kms_http_ep_server_stop_impl;

  /* Registers a private structure for an instantiatable type */
  g_type_class_add_private (klass, sizeof (KmsHttpEPServerPrivate) );
}

static void
kms_http_ep_server_init (KmsHttpEPServer *self)
{
  self->priv = KMS_HTTP_EP_SERVER_GET_PRIVATE (self);
}

/* Virtual public methods */
void
kms_http_ep_server_start (KmsHttpEPServer *self)
{
  g_return_if_fail (KMS_IS_HTTP_EP_SERVER (self) );

  KMS_HTTP_EP_SERVER_GET_CLASS (self)->start (self);
}

void
kms_http_ep_server_stop (KmsHttpEPServer *self)
{
  g_return_if_fail (KMS_IS_HTTP_EP_SERVER (self) );

  KMS_HTTP_EP_SERVER_GET_CLASS (self)->stop (self);
}