#ifndef __KMS_CORE_H__
#define __KMS_CORE_H__

#include <glib.h>

#define __KMS_CORE_H_INSIDE__

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

#define DEBUG g_print("%s:%d\n", __FILE__, __LINE__)

#endif /* __KMS_CORE_H__ */
