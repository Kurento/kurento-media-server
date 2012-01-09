#include <kms-core.h>

static void
kms_media_handler_factory_base_init (gpointer g_class) {
	static gboolean is_initialized = FALSE;

	if (!is_initialized) {
		/* add properties and signals to the interface here */

		is_initialized = TRUE;
	}
}

GType
kms_media_handler_factory_get_type(void) {
	static GType iface_type = 0;
	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (KmsMediaHandlerFactoryInterface),
			kms_media_handler_factory_base_init,   /* base_init */
			NULL,   /* base_finalize */
		};

		iface_type = g_type_register_static(G_TYPE_INTERFACE,
						"KmsMediaHandlerFactory",
						&info, 0);
	}

	return iface_type;
}

KmsMediaHandlerSrc*
kms_media_handler_factory_get_src(KmsMediaHandlerFactory *self) {
	KmsMediaHandlerFactoryInterface *iface;

	g_return_val_if_fail(KMS_IS_MEDIA_HANDLER_FACTORY(self), NULL);

	iface = KMS_MEDIA_HANDLER_FACTORY_GET_INTERFACE(self);

	if (iface->get_src != NULL)
		return iface->get_src(self);
	else {
		gchar *msg = g_strdup_printf(
			"%s does not provide and implementation of "
			"get_src method",
			G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
		g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, msg);
		g_free(msg);
		return NULL;
	}
}

KmsMediaHandlerSink*
kms_media_handler_factory_get_sink(KmsMediaHandlerFactory *self) {
	KmsMediaHandlerFactoryInterface *iface;

	g_return_val_if_fail(KMS_IS_MEDIA_HANDLER_FACTORY(self), NULL);

	iface = KMS_MEDIA_HANDLER_FACTORY_GET_INTERFACE(self);

	if (iface->get_sink != NULL)
		return iface->get_sink(self);
	else {
		gchar *msg = g_strdup_printf(
			"%s does not provide and implementation of "
			"get_sink method",
			G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(self)));
		g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, msg);
		g_free(msg);
		return NULL;
	}
}
