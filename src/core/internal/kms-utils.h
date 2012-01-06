#ifndef __KMS_UTILS_H__
#define __KMS_UTILS_H__

#include <gst/gst.h>

#define KMS_DEBUG_PIPE(name) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(		\
				GST_BIN(kms_get_pipeline()),		\
				GST_DEBUG_GRAPH_SHOW_ALL, name);	\

GstElement* kms_get_pipeline();

void kms_dynamic_connection(GstElement *orig, GstElement *dest, const gchar *name);

GstElement* kms_utils_get_element_for_caps(GstElementFactoryListType type,
				GstRank rank, const GstCaps *caps,
				GstPadDirection direction, gboolean subsetonly,
				const gchar *name);

GstElement* kms_generate_bin_with_caps(GstElement *elem, GstCaps *sink_caps,
							GstCaps *src_caps);

#endif /* __KMS_UTILS_H__ */
