#include <kms-core.h>

#define KMS_LOCAL_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_LOCAL_CONNECTION, KmsLocalConnectionPriv))

#define LOCK(obj) (g_static_mutex_lock(&(KMS_LOCAL_CONNECTION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_static_mutex_unlock(&(KMS_LOCAL_CONNECTION(obj)->priv->mutex)))

struct _KmsLocalConnectionPriv {
	GStaticMutex mutex;
	KmsMediaHandlerFactory *media_factory;
	KmsMediaHandlerSrc *src;
	KmsMediaHandlerSink *sink;

	KmsLocalConnection *other_audio;
	KmsLocalConnection *other_video;
};

enum {
	PROP_0,

	PROP_MEDIA_FACTORY
};

G_DEFINE_TYPE(KmsLocalConnection, kms_local_connection, KMS_TYPE_CONNECTION)

static void
other_audio_unref(gpointer data, GObject *other) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(data);

	LOCK(self);
	if (self->priv->other_audio == KMS_LOCAL_CONNECTION(other))
		self->priv->other_audio = NULL;
	UNLOCK(self);
}

static void
other_video_unref(gpointer data, GObject *other) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(data);

	LOCK(self);
	if (self->priv->other_audio == KMS_LOCAL_CONNECTION(other))
		self->priv->other_audio = NULL;
	UNLOCK(self);
}

static void
dispose_other_audio(KmsLocalConnection *self) {
	if (self->priv->other_audio != NULL) {
		g_object_weak_unref(G_OBJECT(self->priv->other_audio),
						other_audio_unref, self);
		self->priv->other_audio = NULL;
	}
}

static void
dispose_other_video(KmsLocalConnection *self) {
	if (self->priv->other_video != NULL) {
		g_object_weak_unref(G_OBJECT(self->priv->other_video),
						other_video_unref, self);
		self->priv->other_video = NULL;
	}
}

static void
dispose_factory(KmsLocalConnection *self) {
	if (self->priv->media_factory != NULL) {
		g_object_unref(G_OBJECT(self->priv->media_factory));
		self->priv->media_factory = NULL;
	}
}

static void
dispose_src(KmsLocalConnection *self) {
	if (self->priv->src != NULL) {
		g_object_unref(self->priv->src);
		self->priv->src = NULL;
	}
}

static void
dispose_sink(KmsLocalConnection *self) {
	if (self->priv->sink != NULL) {
		g_object_unref(self->priv->sink);
		self->priv->sink = NULL;
	}
}

static gboolean
mode_changed(KmsConnection *self, KmsConnectionMode mode, KmsMediaType type,
								GError **err) {
	/* TODO: Implement mode changed for LocalConnection*/
	return TRUE;
}

static gboolean
connect(KmsConnection *self, KmsConnection *other, KmsMediaType type,
								GError **err) {
	/* TODO: Implement this method*/
	KMS_LOG_DEBUG("TODO: Implement this method");
	return TRUE;
}

static void
constructed(GObject *object) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	G_OBJECT_CLASS(kms_local_connection_parent_class)->constructed(object);

	LOCK(self);
	g_return_if_fail(self->priv->media_factory !=NULL);

	self->priv->src = kms_media_handler_factory_get_src(
						self->priv->media_factory);
	self->priv->sink = kms_media_handler_factory_get_sink(
						self->priv->media_factory);
	UNLOCK(self);
}

static void
set_property(GObject  *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	switch (property_id) {
		case PROP_0:
			/* Do nothing */
			break;
		case PROP_MEDIA_FACTORY: {
			gpointer pfactory;
			if (!G_VALUE_HOLDS_POINTER(value))
				break;
			pfactory = g_value_get_pointer(value);
			LOCK(self);
			dispose_factory(self);
			if (KMS_IS_MEDIA_HANDLER_FACTORY(pfactory))
				self->priv->media_factory = g_object_ref(
								pfactory);
			else
				self->priv->media_factory = NULL;
			UNLOCK(self);
			break;
		}
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	switch (property_id) {
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
dispose(GObject *object) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	LOCK(self);
	dispose_factory(self);
	dispose_src(self);
	dispose_sink(self);
	dispose_other_audio(self);
	dispose_other_video(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_local_connection_parent_class)->dispose(object);
}

static void
finalize(GObject *object) {
	KmsLocalConnection *self = KMS_LOCAL_CONNECTION(object);

	g_static_mutex_free(&(self->priv->mutex));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_local_connection_parent_class)->finalize(object);
}

static void
kms_local_connection_class_init (KmsLocalConnectionClass *klass) {
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GParamSpec *pspec;

	g_type_class_add_private (klass, sizeof (KmsLocalConnectionPriv));

	KMS_CONNECTION_CLASS(klass)->mode_changed = mode_changed;
	KMS_CONNECTION_CLASS(klass)->connect = connect;

	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;
	gobject_class->dispose = dispose;
	gobject_class->finalize = finalize;
	gobject_class->constructed = constructed;

	pspec = g_param_spec_pointer("media-factory", "Media handler factory",
					"The media handler factory",
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_WRITABLE);

	g_object_class_install_property(gobject_class, PROP_MEDIA_FACTORY, pspec);
}

static void
kms_local_connection_init (KmsLocalConnection *self) {
	self->priv = KMS_LOCAL_CONNECTION_GET_PRIVATE(self);

	g_static_mutex_init(&(self->priv->mutex));

	self->priv->media_factory = NULL;
	self->priv->src = NULL;
	self->priv->sink = NULL;
	self->priv->other_video = NULL;
	self->priv->other_audio = NULL;
}
