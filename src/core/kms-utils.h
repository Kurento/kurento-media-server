#if !defined (__KMS_CORE_H_INSIDE__)
#error "Only <kms-core.h> can be included directly."
#endif

#ifndef __KMS_UTILS_H__
#define __KMS_UTILS_H__

#include <gst/gst.h>

void kms_init(gint *argc, gchar **argv[]);
GstElement* kms_get_pipeline();

#endif /* __KMS_UTILS_H__ */