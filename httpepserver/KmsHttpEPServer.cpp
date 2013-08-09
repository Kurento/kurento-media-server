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

#include <libsoup/soup.h>
#include <uuid/uuid.h>

#include "KmsHttpEPServer.h"

#define OBJECT_NAME "HttpEPServer"

/* 36-byte string (plus tailing '\0') */
#define UUID_STR_SIZE 37

#define HTTP_EP_SERVER_ROOT_PATH "/"

#define GST_CAT_DEFAULT kms_http_ep_server_debug_category
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define KMS_HTTP_EP_SERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), KMS_TYPE_HTTP_EP_SERVER, KmsHttpEPServerPrivate))
struct _KmsHttpEPServerPrivate {
  SoupServer *server;
  GSList *handlers;
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

/* signals */
enum {
  URL_REMOVED,
  LAST_SIGNAL
};

static guint obj_signals[LAST_SIGNAL] = { 0 };

struct handler_data {
  gpointer data;
  GDestroyNotify destroy;
  KmsHttpEPServer *server;
};

static void
kms_http_ep_server_remove_handler (const gchar *url, KmsHttpEPServer *self)
{
  GST_DEBUG ("Remove url: %s", url);
  soup_server_remove_handler (self->priv->server, url);

  g_signal_emit (G_OBJECT (self), obj_signals[URL_REMOVED], 0, url);
}

static void
kms_http_ep_server_req_handler (SoupServer *server, SoupMessage *msg,
    const char *path, GHashTable *query, SoupClientContext *client,
    gpointer user_data)
{
  struct handler_data *hdata = (struct handler_data *) user_data;
  gchar *url;
  GSList *l;

  GST_WARNING ("%s path: %s", msg->method, path);

  if (g_strcmp0 (path, HTTP_EP_SERVER_ROOT_PATH) == 0) {
    soup_message_set_status_full (msg, SOUP_STATUS_NOT_IMPLEMENTED,
        "Not implemented");
    return;
  }

  l = g_slist_find_custom (hdata->server->priv->handlers, path,
      (GCompareFunc) g_strcmp0);

  if (l == NULL) {
    /* URL is not registered */
    soup_message_set_status_full (msg, SOUP_STATUS_NOT_FOUND,
        "Http end point not found");
    return;
  }

  /* TODO: Send request to the object */
  soup_message_set_status_full (msg, SOUP_STATUS_OK, "Transfer media");

  url = (gchar *) l->data;
  hdata->server->priv->handlers = g_slist_remove (hdata->server->priv->handlers,
      l->data);

  kms_http_ep_server_remove_handler (path, hdata->server);
  g_free (url);
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
  g_slist_foreach (self->priv->handlers,
      (GFunc) kms_http_ep_server_remove_handler, self);

  /* Stops processing for server */
  soup_server_quit (self->priv->server);
}

static void
destroy_handler_data (struct handler_data *hdata)
{
  if (hdata->destroy != NULL) {
    GST_DEBUG ("Destroying handler data: %P", hdata->data);
    hdata->destroy (hdata->data);
  }

  g_object_unref (hdata->server);
  g_slice_free (struct handler_data, hdata);
}

static gboolean
kms_http_ep_server_register_handler (KmsHttpEPServer *self, gchar *url,
    gpointer data, GDestroyNotify destroy)
{
  struct handler_data *hdata;
  GSList *l;

  l = g_slist_find_custom (self->priv->handlers, url, (GCompareFunc) g_strcmp0);

  if (l != NULL) {
    /* URL is already registered */
    return FALSE;
  }

  self->priv->handlers = g_slist_prepend (self->priv->handlers, url);

  hdata = g_slice_new (struct handler_data);
  hdata->data = data;
  hdata->destroy = destroy;
  hdata->server = KMS_HTTP_EP_SERVER (g_object_ref (self) );

  soup_server_add_handler (self->priv->server, url,
      kms_http_ep_server_req_handler, hdata,
      (GDestroyNotify) destroy_handler_data);

  return TRUE;
}

static const gchar *
kms_http_ep_server_register_end_point_impl (KmsHttpEPServer *self,
    gpointer data, GDestroyNotify destroy)
{
  gchar *url;
  uuid_t uuid;
  gchar *uuid_str;

  uuid_str = (gchar *) g_malloc (UUID_STR_SIZE);
  uuid_generate (uuid);
  uuid_unparse (uuid, uuid_str);

  /* Create URL from uuid string and add it to list of handlers */
  url = g_strdup_printf ("/%s", uuid_str);
  g_free (uuid_str);

  if (!kms_http_ep_server_register_handler (self, url, data, destroy) ) {
    g_free (url);
    return NULL;
  }

  return url;
}

static void
kms_http_ep_server_dispose (GObject *obj)
{
  KmsHttpEPServer *self = KMS_HTTP_EP_SERVER (obj);

  GST_DEBUG_OBJECT (self, "dispose");

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

  GST_DEBUG_OBJECT (self, "finalize");

  if (self->priv->iface) {
    g_free (self->priv->iface);
    self->priv->iface = NULL;
  }

  if (self->priv->handlers != NULL) {
    g_slist_free_full (self->priv->handlers, g_free);
    self->priv->handlers = NULL;
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
  klass->register_end_point = kms_http_ep_server_register_end_point_impl;

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

  obj_signals[URL_REMOVED] =
    g_signal_new ("url-removed",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST,
        G_STRUCT_OFFSET (KmsHttpEPServerClass, url_removed), NULL, NULL,
        g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

  /* Registers a private structure for an instantiatable type */
  g_type_class_add_private (klass, sizeof (KmsHttpEPServerPrivate) );
}

static void
kms_http_ep_server_init (KmsHttpEPServer *self)
{
  self->priv = KMS_HTTP_EP_SERVER_GET_PRIVATE (self);

  /* Set default values */
  self->priv->server = NULL;
  self->priv->handlers = NULL;
  self->priv->port = KMS_HTTP_EP_SERVER_DEFAULT_PORT;
  self->priv->iface = KMS_HTTP_EP_SERVER_DEFAULT_INTERFACE;
}

/* Virtual public methods */
KmsHttpEPServer *
kms_http_ep_server_new (const char *optname1, ...)
{
  KmsHttpEPServer *self;
  SoupAddress *addr = NULL;
  gchar *root_path;

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

  root_path = g_strdup_printf (HTTP_EP_SERVER_ROOT_PATH);

  kms_http_ep_server_register_handler (self, root_path, g_object_ref (self),
      g_object_unref);

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

const gchar *
kms_http_ep_server_register_end_point (KmsHttpEPServer *self,
    gpointer data, GDestroyNotify destroy)
{
  g_return_val_if_fail (KMS_IS_HTTP_EP_SERVER (self), NULL);

  return KMS_HTTP_EP_SERVER_GET_CLASS (self)->register_end_point (self, data,
      destroy);
}