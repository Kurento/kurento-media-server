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

/* inclusion guard */
#ifndef __KMS_HTTP_POST_H__
#define __KMS_HTTP_POST_H__

#include <gst/gst.h>
#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_HTTP_POST (            \
  kms_http_post_get_type ()             \
)

#define KMS_HTTP_POST(obj) (            \
  G_TYPE_CHECK_INSTANCE_CAST (          \
    (obj),                              \
    KMS_TYPE_HTTP_POST,                 \
    KmsHttpPost                         \
  )                                     \
)

#define KMS_IS_HTTP_POST(obj) (         \
  G_TYPE_CHECK_INSTANCE_TYPE (          \
    (obj),                              \
    KMS_TYPE_HTTP_POST                  \
  )                                     \
)

#define KMS_HTTP_POST_CLASS(klass) (    \
  G_TYPE_CHECK_CLASS_CAST (             \
    (klass),                            \
    KMS_TYPE_HTTP_POST,                 \
    KmsHttpPostClass                    \
  )                                     \
)

#define KMS_IS_HTTP_POST_CLASS(klass) ( \
  G_TYPE_CHECK_CLASS_TYPE (             \
    (klass),                            \
    KMS_TYPE_HTTP_POST                  \
  )                                     \
)

#define KMS_HTTP_POST_GET_CLASS(obj) (  \
  G_TYPE_INSTANCE_GET_CLASS (           \
    (obj),                              \
    KMS_TYPE_HTTP_POST,                 \
    KmsHttpPostClass)                   \
)

typedef struct _KmsHttpPost KmsHttpPost;
typedef struct _KmsHttpPostClass KmsHttpPostClass;
typedef struct _KmsHttpPostPrivate KmsHttpPostPrivate;

struct _KmsHttpPost
{
  GObject parent_instance;

  /*< private > */
  KmsHttpPostPrivate *priv;
};

struct _KmsHttpPostClass
{
  GObjectClass parent_class;

  /* signal callbacks */
  void (*got_data) (KmsHttpPost * self, SoupBuffer *buffer);
  void (*finished) (KmsHttpPost * self);
};

/* used by KMS_TYPE_HTTP_POST */
GType kms_http_post_get_type (void);

KmsHttpPost * kms_http_post_new ();

#endif /* __KMS_HTTP_POST_H__ */