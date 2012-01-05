#ifndef __KMS_UTILS_H__
#define __KMS_UTILS_H__

#include <gst/gst.h>

GstElement* kms_get_pipeline();

void kms_dynamic_connection(GstElement *orig, GstElement *dest, const gchar *name);

#endif /* __KMS_UTILS_H__ */
