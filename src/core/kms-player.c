/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
