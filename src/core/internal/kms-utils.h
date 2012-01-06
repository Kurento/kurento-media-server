#ifndef __KMS_UTILS_H__
#define __KMS_UTILS_H__

#include <gst/gst.h>

GstElement* kms_get_pipeline();

void kms_dynamic_connection(GstElement *orig, GstElement *dest, const gchar *name);

GstElement* kms_utils_get_element_for_caps(GstElementFactoryListType type,
				GstRank rank, const GstCaps *caps,
				GstPadDirection direction, gboolean subsetonly,
				const gchar *name);

#endif /* __KMS_UTILS_H__ */
