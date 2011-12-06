#include "kms-media-handler-manager.h"

static void
kms_media_handler_manager_base_init (gpointer g_class) {
	static gboolean is_initialized = FALSE;

	if (!is_initialized) {
		/* add properties and signals to the interface here */

		is_initialized = TRUE;
	}
}

GType
kms_media_handler_manager_get_type(void) {
	static GType iface_type = 0;
	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (KmsMediaHandlerManagerInterface),
			kms_media_handler_manager_base_init,   /* base_init */
			NULL,   /* base_finalize */
		};

		iface_type = g_type_register_static(G_TYPE_INTERFACE,
						"KmsMediaHandlerManager",
						&info, 0);
	}

	return iface_type;
}

KmsMediaHandlerFactory*
kms_media_handler_manager_get_factory(KmsMediaHandlerManager *self) {
	KmsMediaHandlerManagerInterface *iface;

	g_return_if_fail(KMS_IS_MEDIA_HANDLER_MANAGER (self));

	iface = KMS_MEDIA_HANDLER_MANAGER_GET_INTERFACE(self);

	if (iface->get_factory != NULL)
		return iface->get_factory(self);
	else {
		gchar *msg = g_strdup_printf(
				"%s does not provide and implementation of "
				"get_factory method", G_INTERFACE_NAME(iface));
		g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, msg);
		g_free(msg);
		return NULL;
	}
}
