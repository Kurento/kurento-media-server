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

void
kms_media_handler_manager_get_factory(KmsMediaHandlerManager *self) {
	g_return_if_fail(KMS_IS_MEDIA_HANDLER_MANAGER (self));

	KMS_MEDIA_HANDLER_MANAGER_GET_INTERFACE(self)->get_factory(self);
}
