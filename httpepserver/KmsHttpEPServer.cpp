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
#include <libsoup/soup.h>

#include "KmsHttpEPServer.h"

#define OBJECT_NAME "HttpEPServer"

#define HTTP_EP_SERVER_ROOT_PATH "/"

#define GST_CAT_DEFAULT kms_http_ep_server_debug_category
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define KMS_HTTP_EP_SERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_HTTP_EP_SERVER, KmsHttpEPServerPrivate))
struct _KmsHttpEPServerPrivate {
  SoupServer *server;
  gchar *iface;
  gint port;
};

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (KmsHttpEPServer, kms_http_ep_server,
    G_TYPE_OBJECT,
    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, OBJECT_NAME,
        0, "debug category for " OBJECT_NAME " element") )

/* properties */
enum {
  PROP_0,

  PROP_KMS_HTTP_EP_SERVER_PORT,
  PROP_KMS_HTTP_EP_SERVER_INTERFACE,

  N_PROPERTIES
};

#define KMS_HTTP_EP_SERVER_DEFAULT_PORT 0
#define KMS_HTTP_EP_SERVER_DEFAULT_INTERFACE NULL

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
kms_http_ep_server_remove_handlers (KmsHttpEPServer *self)
{
  /* So far, we have just one handler */
  soup_server_remove_handler (self->priv->server, HTTP_EP_SERVER_ROOT_PATH);
}

static void
kms_http_ep_server_start_impl (KmsHttpEPServer *self)
{
  SoupSocket *listener;
  SoupAddress *addr;

  soup_server_run_async (self->priv->server);

  listener = soup_server_get_listener (self->priv->server);

  if (!soup_socket_is_connected (listener) ) {
    GST_ERROR ("Server socket is not connected");
    return;
  }

  addr = soup_socket_get_local_address (listener);
  GST_DEBUG ("Http end point server running in %s:%d",
      soup_address_get_physical (addr), soup_address_get_port (addr) );
}

static void
kms_http_ep_server_stop_impl (KmsHttpEPServer *self)
{
  /* Remove handlers */
  kms_http_ep_server_remove_handlers (self);

  /* Stops processing for server */
  soup_server_quit (self->priv->server);
}

static void
kms_http_ep_server_dispose (GObject *obj)
{
  KmsHttpEPServer *self = KMS_HTTP_EP_SERVER (obj);

  if (self->priv->server) {
    soup_server_disconnect (self->priv->server);
    g_clear_object (&self->priv->server);
  }

  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_ep_server_parent_class)->dispose (obj);
}

static void
kms_http_ep_server_finalize (GObject *obj)
{
  KmsHttpEPServer *self = KMS_HTTP_EP_SERVER (obj);

  if (self->priv->iface) {
    g_free (self->priv->iface);
    self->priv->iface = NULL;
  }

  /* Chain up to the parent class */
  G_OBJECT_CLASS (kms_http_ep_server_parent_class)->finalize (obj);
}

static void
kms_http_ep_server_set_property (GObject *obj, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
  KmsHttpEPServer *self = KMS_HTTP_EP_SERVER (obj);

  switch (prop_id) {
  case PROP_KMS_HTTP_EP_SERVER_PORT:
    self->priv->port = g_value_get_int (value);
    break;
  case PROP_KMS_HTTP_EP_SERVER_INTERFACE:

    if (self->priv->iface != NULL)
      g_free (self->priv->iface);

    self->priv->iface = g_value_dup_string (value);
    break;
  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    break;
  }
}

static void
kms_http_ep_server_get_property (GObject *obj, guint prop_id, GValue *value,
    GParamSpec *pspec)
{
  KmsHttpEPServer *self = KMS_HTTP_EP_SERVER (obj);

  switch (prop_id) {
  case PROP_KMS_HTTP_EP_SERVER_PORT:
    g_value_set_int (value, self->priv->port);
    break;
  case PROP_KMS_HTTP_EP_SERVER_INTERFACE:
    g_value_set_string (value, self->priv->iface);
    break;
  default:
    /* We don't have any other property... */
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
    break;
  }
}

static void
kms_http_ep_server_class_init (KmsHttpEPServerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = kms_http_ep_server_set_property;
  gobject_class->get_property = kms_http_ep_server_get_property;
  gobject_class->dispose = kms_http_ep_server_dispose;
  gobject_class->finalize = kms_http_ep_server_finalize;

  /* Set public virtual methods */
  klass->start = kms_http_ep_server_start_impl;
  klass->stop = kms_http_ep_server_stop_impl;

  obj_properties[PROP_KMS_HTTP_EP_SERVER_PORT] =
    g_param_spec_int ("port",
        "port number",
        "The TCP port to listen on",
        0,
        G_MAXUSHORT,
        KMS_HTTP_EP_SERVER_DEFAULT_PORT,
        (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE) );

  obj_properties[PROP_KMS_HTTP_EP_SERVER_INTERFACE] =
    g_param_spec_string ("interface",
        "IP address",
        "IP address of the network interface to run the server on",
        KMS_HTTP_EP_SERVER_DEFAULT_INTERFACE,
        (GParamFlags) (G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE) );

  g_object_class_install_properties (gobject_class,
      N_PROPERTIES,
      obj_properties);

  /* Registers a private structure for an instantiatable type */
  g_type_class_add_private (klass, sizeof (KmsHttpEPServerPrivate) );
}

static void
kms_http_ep_server_init (KmsHttpEPServer *self)
{
  self->priv = KMS_HTTP_EP_SERVER_GET_PRIVATE (self);

  /* Set default values */
  self->priv->server = NULL;
  self->priv->port = KMS_HTTP_EP_SERVER_DEFAULT_PORT;
  self->priv->iface = KMS_HTTP_EP_SERVER_DEFAULT_INTERFACE;
}

static void
kms_http_ep_server_req_handler (SoupServer *server, SoupMessage *msg,
    const char *path, GHashTable *query, SoupClientContext *client,
    gpointer user_data)
{
  GST_WARNING ("%s path: %s", msg->method, path);

  soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED,
      "HttpEPServer is not yet implemented");
}

/* Virtual public methods */
KmsHttpEPServer *
kms_http_ep_server_new (const char *optname1, ...)
{
  KmsHttpEPServer *self;
  SoupAddress *addr = NULL;
  va_list ap;

  va_start (ap, optname1);
  self = KMS_HTTP_EP_SERVER (g_object_new_valist (KMS_TYPE_HTTP_EP_SERVER,
      optname1, ap) );
  va_end (ap);

  if (self->priv->iface != NULL) {
    addr = soup_address_new (self->priv->iface, self->priv->port);
    /* Synchronously resolves the missing half of addr */
    /* FIXME: Change this to resolv the address asynchronously */
    soup_address_resolve_sync (addr, NULL);
  }

  self->priv->server = soup_server_new (SOUP_SERVER_PORT, self->priv->port,
      SOUP_SERVER_INTERFACE, addr, NULL);
  soup_server_add_handler (self->priv->server, HTTP_EP_SERVER_ROOT_PATH,
      kms_http_ep_server_req_handler, g_object_ref (self), g_object_unref);

  return KMS_HTTP_EP_SERVER (self);
}

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