#include <kms-core.h>

static void
kms_media_handler_sink_base_init (gpointer g_class) {
	static gboolean is_initialized = FALSE;

	if (!is_initialized) {
		/* add properties and signals to the interface here */

		is_initialized = TRUE;
	}
}

GType
kms_media_handler_sink_get_type(void) {
	static GType iface_type = 0;
	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (KmsMediaHandlerSinkInterface),
			kms_media_handler_sink_base_init,   /* base_init */
			NULL,   /* base_finalize */
		};

		iface_type = g_type_register_static(G_TYPE_INTERFACE,
						"KmsMediaHandlerSink",
						&info, 0);
	}

	return iface_type;
}
