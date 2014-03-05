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
#ifndef _KMS_HTTP_LOOP_H_
#define _KMS_HTTP_LOOP_H_

G_BEGIN_DECLS
#define KMS_TYPE_HTTP_LOOP (kms_http_loop_get_type())
#define KMS_HTTP_LOOP(obj) (               \
  G_TYPE_CHECK_INSTANCE_CAST (             \
    (obj),                                 \
    KMS_TYPE_HTTP_LOOP,                    \
    KmsHttpLoop                            \
  )                                        \
)
#define KMS_HTTP_LOOP_CLASS(klass) (       \
  G_TYPE_CHECK_CLASS_CAST (                \
    (klass),                               \
    KMS_TYPE_HTTP_LOOP,                    \
    KmsHttpLoopClass                       \
  )                                        \
)
#define KMS_IS_HTTP_LOOP(obj) (            \
  G_TYPE_CHECK_INSTANCE_TYPE (             \
    (obj),                                 \
    KMS_TYPE_HTTP_LOOP                     \
  )                                        \
)
#define KMS_IS_HTTP_LOOP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), KMS_TYPE_HTTP_LOOP))
#define KMS_HTTP_LOOP_GET_CLASS(obj) (     \
  G_TYPE_INSTANCE_GET_CLASS (              \
    (obj),                                 \
    KMS_TYPE_HTTP_LOOP,                    \
    KmsHttpLoopClass                       \
  )                                        \
)

typedef struct _KmsHttpLoop KmsHttpLoop;
typedef struct _KmsHttpLoopClass KmsHttpLoopClass;
typedef struct _KmsHttpLoopPrivate KmsHttpLoopPrivate;

struct _KmsHttpLoop
{
  GObject parent;

  /*< private > */
  KmsHttpLoopPrivate *priv;
};

struct _KmsHttpLoopClass
{
  GObjectClass parent_class;
};

GType kms_http_loop_get_type (void);

KmsHttpLoop * kms_http_loop_new (void);

guint kms_http_loop_idle_add (KmsHttpLoop *self, GSourceFunc function,
  gpointer data);

guint kms_http_loop_idle_add_full (KmsHttpLoop *self, gint priority,
  GSourceFunc function, gpointer data, GDestroyNotify notify);

guint kms_http_loop_timeout_add (KmsHttpLoop *self, guint interval, GSourceFunc function,
  gpointer data);

guint kms_http_loop_timeout_add_full (KmsHttpLoop *self, gint priority, guint interval,
  GSourceFunc function, gpointer data, GDestroyNotify notify);

gboolean kms_http_loop_remove (KmsHttpLoop *self, guint source_id);

#define KMS_HTTP_LOOP_IS_CURRENT_THREAD(loop) \
  kms_http_loop_is_current_thread(loop)

gboolean kms_http_loop_is_current_thread (KmsHttpLoop *self);

G_END_DECLS
#endif