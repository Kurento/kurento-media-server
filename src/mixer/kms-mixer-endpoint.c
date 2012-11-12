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

#include <mixer/kms-mixer.h>
#include <mixer/kms-mixer-manager.h>

#define KMS_MIXER_ENDPOINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_MIXER_ENDPOINT, KmsMixerEndpointPriv))

#define LOCK(obj) (g_mutex_lock(&(KMS_MIXER_ENDPOINT(obj)->priv->mutex)))
#define UNLOCK(obj) (g_mutex_unlock(&(KMS_MIXER_ENDPOINT(obj)->priv->mutex)))

struct _KmsMixerEndpointPriv {
	GMutex mutex;

	KmsMixerManager *manager;
};

enum {
	PROP_0,
};

G_DEFINE_TYPE(KmsMixerEndpoint, kms_mixer_endpoint, KMS_TYPE_ENDPOINT)

static void
dispose_manager(KmsMixerEndpoint *self) {
	if (self->priv->manager != NULL) {
		g_object_unref(self->priv->manager);
		self->priv->manager = NULL;
	}
}

static void
constructed(GObject *object) {
	KmsMixerEndpoint *self = KMS_MIXER_ENDPOINT(object);
	KmsMixerManager *manager;
	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_endpoint_parent_class)->constructed(object);

	LOCK(self);
	manager = g_object_new(KMS_TYPE_MIXER_MANAGER, NULL);
	g_object_set(object, "manager", manager, NULL);
	dispose_manager(self);
	self->priv->manager = manager;
	UNLOCK(self);
}

static void
dispose(GObject *object) {
	KmsMixerEndpoint *self = KMS_MIXER_ENDPOINT(object);

	LOCK(self);
	dispose_manager(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_endpoint_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsMixerEndpoint *self = KMS_MIXER_ENDPOINT(object);

	g_mutex_clear(&self->priv->mutex);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_mixer_endpoint_parent_class)->finalize(object);
}

static void
kms_mixer_endpoint_class_init(KmsMixerEndpointClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	g_type_class_add_private(klass, sizeof(KmsMixerEndpointPriv));

	gobject_class->constructed = constructed;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
}

static void
kms_mixer_endpoint_init(KmsMixerEndpoint *self) {
	self->priv = KMS_MIXER_ENDPOINT_GET_PRIVATE(self);

	g_mutex_init(&self->priv->mutex);
	self->priv->manager = NULL;
}
