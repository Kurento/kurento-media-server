#include <kms-core.h>

static void
kms_resource_interface_init(gpointer g_class) {
}

GType
kms_resource_get_type(void) {
	static GType iface_type = 0;
	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (KmsResourceInterface),
			kms_resource_interface_init,   /* base_init */
			NULL,   /* base_finalize */
		};

		iface_type = g_type_register_static(G_TYPE_INTERFACE,
							"KmsResourceInterface",
							&info, 0);
	}

	return iface_type;
}
