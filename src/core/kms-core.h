#ifndef __KMS_CORE_H__
#define __KMS_CORE_H__

#include <glib.h>

#define __KMS_CORE_H_INSIDE__

#include <kms-resource.h>
#include <kms-player.h>
#include <kms-endpoint.h>
#include <kms-connection.h>
#include <kms-enums.h>
#include <kms-local-connection.h>
#include <kms-media-handler-manager.h>
#include <kms-media-handler-factory.h>
#include <kms-media-handler-sink.h>
#include <kms-media-handler-src.h>
#include <kms-sdp-media.h>
#include <kms-sdp-payload.h>
#include <kms-sdp-session.h>

#undef __KMS_CORE_H_INSIDE__

G_BEGIN_DECLS

void kms_init(gint *argc, gchar **argv[]);

gboolean kms_g_ptr_array_contains(GPtrArray *array, gpointer value);

G_END_DECLS

#define KMS_DEBUG g_print("%s:%d\n", __FILE__, __LINE__)

#define KMS_LOG_DEBUG(...) g_print("%s:%d: %s\n", __FILE__, __LINE__, __VA_ARGS__)

#define SET_ERROR(err, quark, type, ...)			\
do {								\
	if (err != NULL && *err == NULL)			\
		*err = g_error_new(quark, type, __VA_ARGS__);	\
} while (FALSE)

#endif /* __KMS_CORE_H__ */
