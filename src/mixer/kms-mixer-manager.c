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

#include <mixer/kms-mixer-manager.h>
#include <mixer/kms-mixer-factory.h>

#define KMS_MIXER_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_MIXER_MANAGER, KmsMixerManagerPriv))

#define LOCK(obj) (g_mutex_lock(&(KMS_MIXER_MANAGER(obj)->priv->mutex)))
#define UNLOCK(obj) (g_mutex_unlock(&(KMS_MIXER_MANAGER(obj)->priv->mutex)))

struct _KmsMixerManagerPriv {
	GMutex mutex;

	GSList *handlers;
};

enum {
	PROP_0,
};

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsMixerManager, kms_mixer_manager, G_TYPE_OBJECT,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init))

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *manager) {
	KmsMixerManager *self = KMS_MIXER_MANAGER(manager);
	KmsMixerFactory *factory;
	GSList *l;

	factory = g_object_new(KMS_TYPE_MIXER_FACTORY, NULL);
	LOCK(self);
	l = self->priv->handlers;
	while (l != NULL) {
		kms_mixer_factory_connect(factory, l->data);
		l = l->next;
	}
	self->priv->handlers = g_slist_prepend(self->priv->handlers,
							g_object_ref(factory));
	UNLOCK(self);

	return KMS_MEDIA_HANDLER_FACTORY(factory);
}

static void
dispose_factory(KmsMediaHandlerManager *manager,
					KmsMediaHandlerFactory *factory) {
	KmsMixerManager *self = KMS_MIXER_MANAGER(manager);

	g_return_if_fail(KMS_IS_MIXER_MANAGER(self));
	g_return_if_fail(KMS_IS_MIXER_FACTORY(factory));

	LOCK(self);
	self->priv->handlers = g_slist_remove(self->priv->handlers, factory);
	kms_mixer_factory_dispose(KMS_MIXER_FACTORY(factory));
	g_object_unref(G_OBJECT(factory));
	UNLOCK(self);
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
	iface->dispose_factory = dispose_factory;
}

static void
dispose_mixer_factory(gpointer factory) {
	kms_mixer_factory_dispose(factory);
	g_object_unref(factory);
}

static void
dispose_handlers(KmsMixerManager *self) {
	if (self->priv->handlers != NULL) {
		g_slist_free_full(self->priv->handlers, dispose_mixer_factory);
		self->priv->handlers = NULL;
	}
}

static void
constructed(GObject *object) {
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_manager_parent_class)->constructed(object);
}

static void
dispose(GObject *object) {
	KmsMixerManager *self = KMS_MIXER_MANAGER(object);

	dispose_handlers(self);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_manager_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerManager *self = KMS_MIXER_MANAGER(object);

	g_mutex_clear(&self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_manager_parent_class)->finalize(object);
}

static void
kms_mixer_manager_class_init(KmsMixerManagerClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsMixerManagerPriv));

	gobject_class->constructed = constructed;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
}

static void
kms_mixer_manager_init(KmsMixerManager *self) {
	self->priv = KMS_MIXER_MANAGER_GET_PRIVATE(self);

	g_mutex_init(&self->priv->mutex);
	self->priv->handlers = NULL;
}
