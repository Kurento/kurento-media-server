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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>

#include "KmsHttpLoop.h"

#define NAME "httploop"

GST_DEBUG_CATEGORY_STATIC (kms_http_loop_debug_category);
#define GST_CAT_DEFAULT kms_http_loop_debug_category

G_DEFINE_TYPE_WITH_CODE (KmsHttpLoop, kms_http_loop,
                         G_TYPE_OBJECT,
                         GST_DEBUG_CATEGORY_INIT (kms_http_loop_debug_category,
                             NAME, 0, "debug category for kurento loop") )

#define KMS_HTTP_LOOP_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_HTTP_LOOP, KmsHttpLoopPrivate))

struct _KmsHttpLoopPrivate {
  GThread *thread;
  GRecMutex rmutex;
  GMainLoop *loop;
  GMainContext *context;
  GCond cond;
  GMutex mutex;
  gboolean initialized;
};

#define KMS_HTTP_LOOP_LOCK(elem) \
  (g_rec_mutex_lock (&KMS_HTTP_LOOP ((elem))->priv->rmutex))
#define KMS_HTTP_LOOP_UNLOCK(elem) \
  (g_rec_mutex_unlock (&KMS_HTTP_LOOP ((elem))->priv->rmutex))

/* Object properties */
enum {
  PROP_0,
  PROP_CONTEXT,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static gboolean
quit_main_loop (KmsHttpLoop *self)
{
  GST_DEBUG ("Exiting main loop");

  g_main_loop_quit (self->priv->loop);

  return G_SOURCE_REMOVE;
}

static gpointer
loop_thread_init (gpointer data)
{
  KmsHttpLoop *self = KMS_HTTP_LOOP (data);
  GMainLoop *loop;
  GMainContext *context;

  KMS_HTTP_LOOP_LOCK (self);
  self->priv->context = g_main_context_new ();
  context = g_main_context_ref (self->priv->context);
  self->priv->loop = g_main_loop_new (context, FALSE);
  loop = g_main_loop_ref (self->priv->loop);
  KMS_HTTP_LOOP_UNLOCK (self);

  /* unlock main process because context is already initialized */
  g_mutex_lock (&self->priv->mutex);
  self->priv->initialized = TRUE;
  g_cond_signal (&self->priv->cond);
  g_mutex_unlock (&self->priv->mutex);

  if (!g_main_context_acquire (context) ) {
    GST_ERROR ("Can not acquire context");
    goto end;
  }

  GST_DEBUG ("Running main loop");
  g_main_loop_run (loop);
  g_main_context_release (context);

end:
  GST_DEBUG ("Thread finished");
  g_main_loop_unref (loop);
  g_main_context_unref (context);

  return NULL;
}

static void
kms_http_loop_get_property (GObject *object, guint property_id, GValue *value,
                            GParamSpec *pspec)
{
  KmsHttpLoop *self = KMS_HTTP_LOOP (object);

  KMS_HTTP_LOOP_LOCK (self);

  switch (property_id) {
  case PROP_CONTEXT:
    g_value_set_boxed (value, self->priv->context);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }

  KMS_HTTP_LOOP_UNLOCK (self);
}

static void
kms_http_loop_dispose (GObject *obj)
{
  KmsHttpLoop *self = KMS_HTTP_LOOP (obj);

  GST_DEBUG_OBJECT (obj, "Dispose");

  KMS_HTTP_LOOP_LOCK (self);

  if (self->priv->thread != NULL) {
    if (g_thread_self () != self->priv->thread) {
      GThread *aux = self->priv->thread;

      kms_http_loop_idle_add (self, (GSourceFunc) quit_main_loop,
                              self);
      self->priv->thread = NULL;
      KMS_HTTP_LOOP_UNLOCK (self);
      g_thread_join (aux);
      KMS_HTTP_LOOP_LOCK (self);
    } else {
      /* self thread does not need to wait for itself */
      quit_main_loop (self);
      g_thread_unref (self->priv->thread);
      self->priv->thread = NULL;
    }
  }

  KMS_HTTP_LOOP_UNLOCK (self);

  G_OBJECT_CLASS (kms_http_loop_parent_class)->dispose (obj);
}

static void
kms_http_loop_finalize (GObject *obj)
{
  KmsHttpLoop *self = KMS_HTTP_LOOP (obj);

  GST_DEBUG_OBJECT (obj, "Finalize");

  if (self->priv->context != NULL) {
    g_main_context_unref (self->priv->context);
  }

  if (self->priv->loop != NULL) {
    g_main_loop_unref (self->priv->loop);
  }

  g_rec_mutex_clear (&self->priv->rmutex);
  g_mutex_clear (&self->priv->mutex);
  g_cond_clear (&self->priv->cond);

  G_OBJECT_CLASS (kms_http_loop_parent_class)->finalize (obj);
}

static void
kms_http_loop_class_init (KmsHttpLoopClass *klass)
{
  GObjectClass *objclass = G_OBJECT_CLASS (klass);

  objclass->dispose = kms_http_loop_dispose;
  objclass->finalize = kms_http_loop_finalize;
  objclass->get_property = kms_http_loop_get_property;

  /* Install properties */
  obj_properties[PROP_CONTEXT] = g_param_spec_boxed ("context",
                                 "Main loop context",
                                 "Main loop context",
                                 G_TYPE_MAIN_CONTEXT,
                                 (GParamFlags) (G_PARAM_READABLE) );

  g_object_class_install_properties (objclass, N_PROPERTIES, obj_properties);

  /* Registers a private structure for the instantiatable type */
  g_type_class_add_private (klass, sizeof (KmsHttpLoopPrivate) );
}

static void
kms_http_loop_init (KmsHttpLoop *self)
{
  self->priv = KMS_HTTP_LOOP_GET_PRIVATE (self);
  self->priv->context = NULL;
  self->priv->loop = NULL;
  g_rec_mutex_init (&self->priv->rmutex);
  g_cond_init (&self->priv->cond);
  g_mutex_init (&self->priv->mutex);

  self->priv->thread = g_thread_new ("KmsHttpLoop", loop_thread_init, self);

  g_mutex_lock (&self->priv->mutex);

  while (!self->priv->initialized) {
    g_cond_wait (&self->priv->cond, &self->priv->mutex);
  }

  g_mutex_unlock (&self->priv->mutex);
}

KmsHttpLoop *
kms_http_loop_new (void)
{
  return KMS_HTTP_LOOP (g_object_new (KMS_TYPE_HTTP_LOOP, NULL) );
}

static guint
kms_http_loop_attach (KmsHttpLoop *self, GSource *source, gint priority,
                      GSourceFunc function, gpointer data,
                      GDestroyNotify notify)
{
  guint id;

  KMS_HTTP_LOOP_LOCK (self);

  if (self->priv->thread == NULL) {
    KMS_HTTP_LOOP_UNLOCK (self);
    return 0;
  }

  g_source_set_priority (source, priority);
  g_source_set_callback (source, function, data, notify);
  id = g_source_attach (source, self->priv->context);

  KMS_HTTP_LOOP_UNLOCK (self);

  return id;
}

guint
kms_http_loop_idle_add_full (KmsHttpLoop *self, gint priority,
                             GSourceFunc function,
                             gpointer data, GDestroyNotify notify)
{
  GSource *source;
  guint id;

  if (!KMS_IS_HTTP_LOOP (self) ) {
    return 0;
  }

  source = g_idle_source_new ();
  id = kms_http_loop_attach (self, source, priority, function, data, notify);
  g_source_unref (source);

  return id;
}

guint
kms_http_loop_idle_add (KmsHttpLoop *self, GSourceFunc function, gpointer data)
{
  return kms_http_loop_idle_add_full (self, G_PRIORITY_DEFAULT_IDLE, function,
                                      data,
                                      NULL);
}

guint
kms_http_loop_timeout_add_full (KmsHttpLoop *self, gint priority,
                                guint interval,
                                GSourceFunc function, gpointer data,
                                GDestroyNotify notify)
{
  GSource *source;
  guint id;

  if (!KMS_IS_HTTP_LOOP (self) ) {
    return 0;
  }

  source = g_timeout_source_new (interval);
  id = kms_http_loop_attach (self, source, priority, function, data, notify);
  g_source_unref (source);

  return id;
}

guint
kms_http_loop_timeout_add (KmsHttpLoop *self, guint interval,
                           GSourceFunc function,
                           gpointer data)
{
  return kms_http_loop_timeout_add_full (self, G_PRIORITY_DEFAULT, interval,
                                         function, data, NULL);
}

gboolean
kms_http_loop_remove (KmsHttpLoop *self, guint source_id)
{
  GSource *source;

  source = g_main_context_find_source_by_id (self->priv->context, source_id);

  if (source == NULL) {
    return FALSE;
  }

  g_source_destroy (source);
  return TRUE;
}

gboolean
kms_http_loop_is_current_thread (KmsHttpLoop *self)
{
  return (g_thread_self() == self->priv->thread);
}
