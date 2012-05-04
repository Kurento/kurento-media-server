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

	g_return_val_if_fail(KMS_IS_MEDIA_HANDLER_MANAGER (self), NULL);

	iface = KMS_MEDIA_HANDLER_MANAGER_GET_INTERFACE(self);

	if (iface->get_factory != NULL)
		return iface->get_factory(self);
	else {
		gchar *msg = g_strdup_printf(
				"%s does not provide and implementation of "
				"get_factory method",
				G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
		g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, msg);
		g_free(msg);
		return NULL;
	}
}

void
kms_media_handler_manager_dispose_factory(KmsMediaHandlerManager *self,
					KmsMediaHandlerFactory *factory) {
	KmsMediaHandlerManagerInterface *iface;

	g_return_if_fail(KMS_IS_MEDIA_HANDLER_MANAGER (self));
	g_return_if_fail(KMS_IS_MEDIA_HANDLER_FACTORY(factory));

	iface = KMS_MEDIA_HANDLER_MANAGER_GET_INTERFACE(self);

	if (iface->dispose_factory != NULL) {
		iface->dispose_factory(self, factory);
	} else {
		gchar *msg = g_strdup_printf(
			"%s does not provide and implementation of "
			"dispose_factory method",
			G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
		g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, msg);
		g_free(msg);
	}
}