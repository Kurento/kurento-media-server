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

/* inclusion guard */
#ifndef __KMS_HTTP_EP_SERVER_H__
#define __KMS_HTTP_EP_SERVER_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define KMS_TYPE_HTTP_EP_SERVER (            \
  kms_http_ep_server_get_type ()             \
)

#define KMS_HTTP_EP_SERVER(obj) (            \
  G_TYPE_CHECK_INSTANCE_CAST (               \
    (obj),                                   \
    KMS_TYPE_HTTP_EP_SERVER,                 \
    KmsHttpEPServer                          \
  )                                          \
)

#define KMS_IS_HTTP_EP_SERVER(obj) (         \
  G_TYPE_CHECK_INSTANCE_TYPE (               \
    (obj),                                   \
    KMS_TYPE_HTTP_EP_SERVER                  \
  )                                          \
)

#define KMS_HTTP_EP_SERVER_CLASS(klass) (    \
  G_TYPE_CHECK_CLASS_CAST (                  \
    (klass),                                 \
    KMS_TYPE_HTTP_EP_SERVER,                 \
    KmsHttpEPServerClass                     \
  )                                          \
)

#define KMS_IS_HTTP_EP_SERVER_CLASS(klass) ( \
  G_TYPE_CHECK_CLASS_TYPE (                  \
    (klass),                                 \
    KMS_TYPE_HTTP_EP_SERVER                  \
  )                                          \
)

#define KMS_HTTP_EP_SERVER_GET_CLASS(obj) (  \
  G_TYPE_INSTANCE_GET_CLASS (                \
    (obj),                                   \
    KMS_TYPE_HTTP_EP_SERVER,                 \
    KmsHttpEPServerClass)                    \
)

typedef struct _KmsHttpEPServer KmsHttpEPServer;
typedef struct _KmsHttpEPServerClass KmsHttpEPServerClass;
typedef struct _KmsHttpEPServerPrivate KmsHttpEPServerPrivate;

struct _KmsHttpEPServer
{
  GObject parent_instance;

  /* instance members */

  /*< private > */
  KmsHttpEPServerPrivate *priv;
};

struct _KmsHttpEPServerClass
{
  GObjectClass parent_class;

  /* public virtual methods */
  void (*start) (KmsHttpEPServer * self);
  void (*stop) (KmsHttpEPServer * self);
};

/* used by KMS_TYPE_HTTP_EP_SERVER */
GType kms_http_ep_server_get_type (void);

/* Virtual public methods */
void kms_http_ep_server_start (KmsHttpEPServer * self);
void kms_http_ep_server_stop (KmsHttpEPServer * self);

#endif /* __KMS_HTTP_EP_SERVER_H__ */
