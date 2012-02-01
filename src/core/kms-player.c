#include <kms-core.h>

void
kms_player_set_url(KmsPlayer *self, gchar *url) {
	KMS_PLAYER_GET_INTERFACE(self)->set_url(self, url);
}

void
kms_player_start(KmsPlayer *self) {
	KMS_PLAYER_GET_INTERFACE(self)->start(self);
}

void
kms_player_stop(KmsPlayer *self) {
	KMS_PLAYER_GET_INTERFACE(self)->stop(self);
}

void
kms_player_set_url_default(KmsPlayer *self, gchar *url) {
	g_warning("set_url method not implemented in %s class",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
}

void
kms_player_start_default(KmsPlayer *self) {
	g_warning("start method not implemented in %s class",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
}

void
kms_player_stop_default(KmsPlayer *self) {
	g_warning("stop method not implemented in %s class",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
}

static void
kms_player_interface_init(gpointer g_class) {
	KmsPlayerInterface *iface = (KmsPlayerInterface *) g_class;
	/* add properties and signals to the interface here */

	iface->start = kms_player_start_default;
	iface->set_url = kms_player_set_url_default;
	iface->stop = kms_player_stop_default;
}

GType
kms_player_get_type(void) {
	static GType iface_type = 0;
	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (KmsResourceInterface),
			kms_player_interface_init,   /* base_init */
			NULL,   /* base_finalize */
		};

		iface_type = g_type_register_static(KMS_TYPE_RESOURCE,
							"KmsResourceInterface",
							&info, 0);
	}

	return iface_type;
}
