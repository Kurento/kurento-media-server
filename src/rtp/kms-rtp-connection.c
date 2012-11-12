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
#include <rtp/kms-rtp.h>
#include "internal/kms-utils.h"
#include <nice.h>

#define KMS_RTP_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), KMS_TYPE_RTP_CONNECTION, KmsRtpConnectionPriv))

#define LOCK(obj) (g_mutex_lock(&(KMS_RTP_CONNECTION(obj)->priv->mutex)))
#define UNLOCK(obj) (g_mutex_unlock(&(KMS_RTP_CONNECTION(obj)->priv->mutex)))

#define ICE_TIMEOUT 10
#define STREAM_ID_DATA "stream_id"

enum {
	PROP_0,

	PROP_LOCAL_SPEC,
	PROP_REMOTE_SPEC,
	PROP_DESCRIPTOR,
};

struct _KmsRtpConnectionPriv {
	GMutex mutex;
	KmsSessionSpec *local_spec;
	KmsSessionSpec *remote_spec;
	KmsSessionSpec *neg_local_spec;
	KmsSessionSpec *neg_remote_spec;
	KmsSessionSpec *descriptor;

	NiceAgent *audio_agent;
	NiceAgent *video_agent;
	gboolean audio_gathered;
	gboolean video_gathered;
	gboolean audio_connected;
	gboolean video_connected;
	GMutex ice_mutex;
	GCond ice_cond;

	KmsRtpReceiver *receiver;
	KmsRtpSender *sender;
};

static void media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface);
static void media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface);

G_DEFINE_TYPE_WITH_CODE(KmsRtpConnection, kms_rtp_connection,
				KMS_TYPE_CONNECTION,
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_MANAGER,
					media_handler_manager_iface_init)
				G_IMPLEMENT_INTERFACE(
					KMS_TYPE_MEDIA_HANDLER_FACTORY,
					media_handler_factory_iface_init))

static void
dispose_video_agent(KmsRtpConnection *self) {
	if (self->priv->video_agent != NULL) {
		g_object_unref(self->priv->video_agent);
		self->priv->video_agent = NULL;
	}
}

static void
dispose_audio_agent(KmsRtpConnection *self) {
	if (self->priv->audio_agent != NULL) {
		g_object_unref(self->priv->audio_agent);
		self->priv->audio_agent = NULL;
	}
}

static void
dispose_receiver(KmsRtpConnection *self) {
	if (self->priv->receiver != NULL) {
		kms_media_handler_src_terminate(
				KMS_MEDIA_HANDLER_SRC(self->priv->receiver));
		g_object_unref(self->priv->receiver);
		self->priv->receiver = NULL;
	}
}

static void
dispose_sender(KmsRtpConnection *self) {
	if (self->priv->sender != NULL) {
		kms_media_handler_sink_terminate(
				KMS_MEDIA_HANDLER_SINK(self->priv->sender));
		g_object_unref(self->priv->sender);
		self->priv->sender = NULL;
	}
}

static void
dispose_local_spec(KmsRtpConnection *self) {
	if (self->priv->local_spec != NULL) {
		g_object_unref(self->priv->local_spec);
		self->priv->local_spec = NULL;
	}
}

static void
dispose_remote_spec(KmsRtpConnection *self) {
	if (self->priv->remote_spec != NULL) {
		g_object_unref(self->priv->remote_spec);
		self->priv->remote_spec = NULL;
	}
}

static void
dispose_neg_local_spec(KmsRtpConnection *self) {
	if (self->priv->neg_local_spec != NULL) {
		g_object_unref(self->priv->neg_local_spec);
		self->priv->neg_local_spec = NULL;
	}
}

static void
dispose_neg_remote_spec(KmsRtpConnection *self) {
	if (self->priv->neg_remote_spec != NULL) {
		g_object_unref(self->priv->neg_remote_spec);
		self->priv->neg_remote_spec = NULL;
	}
}

static void
dispose_descriptor(KmsRtpConnection *self) {
	if (self->priv->descriptor != NULL) {
		g_object_unref(self->priv->descriptor);
		self->priv->descriptor = NULL;
	}
}

static KmsMediaHandlerFactory*
get_factory(KmsMediaHandlerManager *iface) {
	return KMS_MEDIA_HANDLER_FACTORY(g_object_ref(iface));
}

static void
dispose_factory(KmsMediaHandlerManager *manager,  KmsMediaHandlerFactory *factory) {
	/* Nothing to do, just avoid the warning */
}

static void
media_handler_manager_iface_init(KmsMediaHandlerManagerInterface *iface) {
	iface->get_factory = get_factory;
	iface->dispose_factory = dispose_factory;
}

static KmsMediaHandlerSrc*
get_src(KmsMediaHandlerFactory *iface) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(iface);
	return KMS_MEDIA_HANDLER_SRC(g_object_ref(self->priv->receiver));
}

static KmsMediaHandlerSink*
get_sink(KmsMediaHandlerFactory *iface) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(iface);
	KmsMediaHandlerSink *sink;

	LOCK(self);
	if (self->priv->sender != NULL)
		sink = KMS_MEDIA_HANDLER_SINK(g_object_ref(self->priv->sender));
	else
		sink = NULL;
	UNLOCK(self);

	return sink;
}

static void
media_handler_factory_iface_init(KmsMediaHandlerFactoryInterface *iface) {
	iface->get_sink = get_sink;
	iface->get_src = get_src;
}

static void
create_rtp_sender(KmsRtpConnection *self) {
	gint audio_fd, video_fd;

	g_object_get(self->priv->receiver, "audio-fd", &audio_fd,
						"video-fd", &video_fd,
						NULL);

	g_object_set(G_OBJECT(self->priv->sender),
					"audio-fd", audio_fd,
					"video-fd", video_fd,
					"audio-agent", self->priv->audio_agent,
					"video-agent", self->priv->video_agent,
					NULL);

	g_object_set(G_OBJECT(self->priv->sender),
				"remote-spec", self->priv->neg_remote_spec,
				NULL);
}

static void
create_media_transmission(KmsRtpConnection *self) {
	g_object_set(G_OBJECT(self->priv->receiver),
					"audio-agent", self->priv->audio_agent,
					"video-agent", self->priv->video_agent,
					NULL);

	g_object_set(G_OBJECT(self->priv->receiver),
					"local-spec", self->priv->descriptor,
					NULL);

	/* Create rtpsender */
	create_rtp_sender(self);
}

static gpointer
create_media_transmission_thread(gpointer data) {
	KmsRtpConnection *self = data;

	g_mutex_lock(&(self->priv->ice_mutex));
	while (!self->priv->audio_connected ||
				!self->priv->video_connected) {
		guint64 end_time = g_get_monotonic_time() +
				ICE_TIMEOUT * G_TIME_SPAN_SECOND;

		if (!g_cond_wait_until(&self->priv->ice_cond,
			&self->priv->ice_mutex, end_time)) {
			g_warning("Timeout waiting for connection");
			g_mutex_unlock(&(self->priv->ice_mutex));
			g_object_unref(self);
			// TODO: notify error to signal plane
			return NULL;
		}
	}
	g_mutex_unlock(&(self->priv->ice_mutex));

	create_media_transmission(self);

	g_object_unref(self);
	return NULL;
}


static void
configure_agents(KmsRtpConnection *self) {
	gint32 i;

	for (i = 0; i < self->priv->remote_spec->medias->len; i++) {
		KmsMediaSpec *media;
		NiceAgent *agent;
		guint stream_id;
		KmsMediaType type;

		media = self->priv->remote_spec->medias->pdata[i];

		if (g_hash_table_lookup(media->type,
					(gpointer) KMS_MEDIA_TYPE_VIDEO)) {
			agent = self->priv->video_agent;
			type = KMS_MEDIA_TYPE_VIDEO;
		} else if (g_hash_table_lookup(media->type,
					(gpointer) KMS_MEDIA_TYPE_AUDIO)) {
			agent = self->priv->audio_agent;
			type = KMS_MEDIA_TYPE_AUDIO;
		} else {
			continue;
		}

		if (agent == NULL)
			continue;

		stream_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(agent),
							STREAM_ID_DATA));

		if (media->transport->__isset_ice) {
			GSList *candidates = NULL;
			gint i;

			g_print("Configuring ICE candidates\n");

			for (i = 0; i < media->transport->ice->candidates->len; i++) {
				KmsTransportIceCandidate *tr_cand;
				NiceCandidate *cand = nice_candidate_new(
						NICE_CANDIDATE_TYPE_HOST);
				if (cand == NULL)
					continue;

				tr_cand = media->transport->ice->candidates->pdata[i];

				nice_address_init(&(cand->addr));
				nice_address_set_from_string(&(cand->addr),
							tr_cand->address);
				nice_address_set_port(&(cand->addr),
							tr_cand->port);

				if (tr_cand->__isset_baseAddress &&
						tr_cand->__isset_basePort) {
					nice_address_init(&(cand->base_addr));
					nice_address_set_from_string(
							&(cand->base_addr),
							tr_cand->baseAddress);
					nice_address_set_port(
							&(cand->base_addr),
							tr_cand->basePort);
				}

				cand->priority = tr_cand->priority;

				g_strlcpy(cand->foundation, tr_cand->foundation,
						sizeof(cand->foundation));

				cand->component_id = tr_cand->componentId;

				if (tr_cand->__isset_streamId)
					cand->stream_id = tr_cand->streamId;
				else
					cand->stream_id = stream_id;

				cand->component_id = tr_cand->componentId;

				nice_agent_set_remote_credentials(agent,
							stream_id,
							tr_cand->username,
							tr_cand->password);

				candidates = g_slist_prepend(candidates, cand);
			}

			nice_agent_set_remote_candidates(agent, stream_id, 1,
							 candidates);

			g_slist_free_full(candidates,
					  (GDestroyNotify) nice_candidate_free);
		} else {
			nice_agent_remove_stream(agent, stream_id);
			if (type == KMS_MEDIA_TYPE_AUDIO)
				dispose_audio_agent(self);
			else if (type == KMS_MEDIA_TYPE_VIDEO)
				dispose_video_agent(self);
			continue;
		}
	}
}

static gboolean
connect_to_remote(KmsConnection *conn, KmsSessionSpec *spec,
					gboolean local_offerer, GError **err) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(conn);

	if (!KMS_IS_SESSION_SPEC(spec)) {
		g_set_error(err, KMS_RTP_CONNECTION_ERROR,
					KMS_RTP_CONNECTION_ERROR_WRONG_VALUE,
					"Given spect has incorrect type");
		return FALSE;
	}

	LOCK(self);
	if (self->priv->remote_spec != NULL) {
		g_set_error(err, KMS_RTP_CONNECTION_ERROR,
					KMS_RTP_CONNECTION_ERROR_ALREADY,
					"Remote spec was already set");
		UNLOCK(self);
		return FALSE;
	}

	self->priv->remote_spec = g_object_ref(spec);

	if (local_offerer) {
		kms_session_spec_intersect(spec, self->priv->local_spec,
						&(self->priv->neg_remote_spec),
						&(self->priv->neg_local_spec));
	} else {
		kms_session_spec_intersect(self->priv->local_spec, spec,
						&(self->priv->neg_local_spec),
						&(self->priv->neg_remote_spec));
	}

	dispose_descriptor(self);
	self->priv->descriptor = g_object_ref(self->priv->neg_local_spec);

	configure_agents(self);

	if (self->priv->audio_agent == NULL || self->priv->video_agent == NULL) {
		dispose_audio_agent(self);
		dispose_video_agent(self);

		create_media_transmission(self);
	} else {
		GThread *t;

		t = g_thread_new("WaitAgents", create_media_transmission_thread,
							g_object_ref(self));
		g_thread_unref(t);
	}

	UNLOCK(self);

	return TRUE;
}

static gboolean
mode_changed(KmsConnection *self, KmsConnectionMode mode, KmsMediaType type,
								GError **err) {
	/* TODO: Implement mode changed for RtpConnection */
	return TRUE;
}

static void
set_property (GObject *object, guint property_id, const GValue *value,
							GParamSpec *pspec) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			dispose_local_spec(self);
			self->priv->local_spec = g_value_dup_object(value);
			if (self->priv->descriptor == NULL &&
					self->priv->local_spec != NULL) {
				self->priv->descriptor = g_object_ref(
							self->priv->local_spec);
			}
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
get_property(GObject *object, guint property_id, GValue *value,
							GParamSpec *pspec) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	switch (property_id) {
		case PROP_LOCAL_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->local_spec);
			UNLOCK(self);
			break;
		case PROP_DESCRIPTOR:
			LOCK(self);
			g_value_set_object(value, self->priv->descriptor);
			UNLOCK(self);
			break;
		case PROP_REMOTE_SPEC:
			LOCK(self);
			g_value_set_object(value, self->priv->neg_remote_spec);
			UNLOCK(self);
			break;
		default:
			/* We don't have any other property... */
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
candidates_ready_audio(NiceAgent *agent, guint stream_id, KmsRtpConnection *self) {
	g_mutex_lock(&(self->priv->ice_mutex));
	self->priv->audio_gathered = TRUE;
	g_cond_signal(&self->priv->ice_cond);
	g_mutex_unlock(&(self->priv->ice_mutex));
}

static void
candidates_ready_video(NiceAgent *agent, guint stream_id, KmsRtpConnection *self) {
	g_mutex_lock(&(self->priv->ice_mutex));
	self->priv->video_gathered = TRUE;
	g_cond_signal(&self->priv->ice_cond);
	g_mutex_unlock(&(self->priv->ice_mutex));
}

static void
state_changed_audio(NiceAgent *agent, guint stream_id, guint component_id,
					guint state, KmsRtpConnection *self) {
	if (state == NICE_COMPONENT_STATE_CONNECTED) {
		g_mutex_lock(&(self->priv->ice_mutex));
		self->priv->audio_connected = TRUE;
		g_cond_signal(&self->priv->ice_cond);
		g_mutex_unlock(&(self->priv->ice_mutex));
	}
}

static void
state_changed_video(NiceAgent *agent, guint stream_id, guint component_id,
					guint state, KmsRtpConnection *self) {
	if (state == NICE_COMPONENT_STATE_CONNECTED) {
		g_mutex_lock(&(self->priv->ice_mutex));
		self->priv->video_connected = TRUE;
		g_cond_signal(&self->priv->ice_cond);
		g_mutex_unlock(&(self->priv->ice_mutex));
	}
}

static void
audio_nice_recv() {
	// Dummy, just used to get ICE candidates
}

static void
video_nice_recv() {
	// Dummy, just used to get ICE candidates
}

static void
create_nice_agents(KmsRtpConnection *self) {
	guint astream_id, vstream_id;

	self->priv->audio_agent = nice_agent_new(g_main_context_default(),
						NICE_COMPATIBILITY_RFC5245);
	self->priv->video_agent = nice_agent_new(g_main_context_default(),
						NICE_COMPATIBILITY_RFC5245);

	GObject *aagent = G_OBJECT(self->priv->audio_agent);

	g_signal_connect(aagent, "candidate-gathering-done",
				G_CALLBACK(candidates_ready_audio), self);
	g_signal_connect(aagent, "component-state-changed",
				G_CALLBACK(state_changed_audio), self);

	g_object_set(aagent, "upnp", FALSE, NULL);

	GObject *vagent = G_OBJECT(self->priv->video_agent);

	g_signal_connect(vagent, "candidate-gathering-done",
				G_CALLBACK(candidates_ready_video), self);
	g_signal_connect(vagent, "component-state-changed",
				G_CALLBACK(state_changed_video), self);

	g_object_set(vagent, "upnp", FALSE, NULL);

// 	g_object_set(aagent, "stun-server", "193.147.51.24", NULL);
// 	g_object_set(aagent, "stun-server-port", (guint) 3478 , NULL);

// 	g_object_set(vagent, "stun-server", "193.147.51.24", NULL);
// 	g_object_set(vagent, "stun-server-port", (guint) 3478 , NULL);

	astream_id = nice_agent_add_stream(self->priv->audio_agent, 1);
	g_object_set_data(aagent, STREAM_ID_DATA, GUINT_TO_POINTER(astream_id));

	if (astream_id == 0)
		goto error;

	vstream_id = nice_agent_add_stream(self->priv->video_agent, 1);
	g_object_set_data(vagent, STREAM_ID_DATA, GUINT_TO_POINTER(vstream_id));

	if (vstream_id == 0)
		goto error;

	nice_agent_attach_recv(self->priv->audio_agent, astream_id, 1,
			       g_main_context_default(), video_nice_recv, NULL);

	nice_agent_attach_recv(self->priv->video_agent, vstream_id, 1,
			       g_main_context_default(), audio_nice_recv, NULL);

	if (!nice_agent_gather_candidates(self->priv->audio_agent, astream_id)) {
		g_print("Error getting candidates audio");
		goto error;
	}

	if (!nice_agent_gather_candidates(self->priv->video_agent, vstream_id)) {
		g_print("Error getting candidates for video");
		goto error;
	}

	g_mutex_lock(&(self->priv->ice_mutex));
	while (!self->priv->audio_gathered || !self->priv->video_gathered) {
		guint64 end_time = g_get_monotonic_time() +
					ICE_TIMEOUT * G_TIME_SPAN_SECOND;

		if (!g_cond_wait_until(&self->priv->ice_cond,
					&self->priv->ice_mutex, end_time)) {
			dispose_audio_agent(self);
			dispose_video_agent(self);
			g_warning("Timeout getting candidates");
			g_mutex_unlock(&(self->priv->ice_mutex));
			return;
		}
	}
	g_mutex_unlock(&(self->priv->ice_mutex));

	return;
error:
	dispose_audio_agent(self);
	dispose_video_agent(self);
	g_warning("Error while initializing ice agents for %s",
							GST_ELEMENT_NAME(self));
}

static void
process_candidates(KmsRtpConnection *self, NiceAgent *agent, KmsMediaType type) {
	GSList *candidates, *l, *final_candidates = NULL;
	guint stream_id;
	guint32 priority = 0;
	gchar addr[NICE_ADDRESS_STRING_LEN];
	gchar aux[NICE_ADDRESS_STRING_LEN];
	gint i, port;

	stream_id = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(agent),
						       STREAM_ID_DATA));

	candidates = nice_agent_get_local_candidates(agent, stream_id, 1);

	for (l = candidates; l != NULL; l = l->next) {
		NiceCandidate *cand = l->data;
		KmsTransportIceCandidate *trcand;

		nice_address_to_string(&(cand->addr), aux);

		if (g_strrstr(aux, ":")) {
			continue;
		}

		trcand = g_object_new(KMS_TYPE_TRANSPORT_ICE_CANDIDATE, NULL);
		if (trcand->address != NULL)
			g_free(trcand->address);
		trcand->address = g_strdup(aux);
		trcand->port = nice_address_get_port(&(cand->addr));
		if (trcand->baseAddress != NULL)
			g_free(trcand->baseAddress);
		nice_address_to_string(&(cand->base_addr), aux);
		trcand->baseAddress = g_strdup(aux);
		trcand->__isset_baseAddress = TRUE;
		trcand->basePort = nice_address_get_port(&(cand->base_addr));
		trcand->__isset_basePort = TRUE;
		trcand->componentId = 1;
		trcand->foundation = g_strdup(cand->foundation);
		trcand->priority = cand->priority;
		trcand->streamId = cand->stream_id;
		trcand->__isset_streamId = TRUE;
		trcand->transport = cand->transport;
		trcand->type = cand->type;

		if (trcand->username != NULL)
			g_free(trcand->username);

		if (trcand->password != NULL)
			g_free(trcand->password);

		nice_agent_get_local_credentials(agent, stream_id,
					&trcand->username, &trcand->password);

		final_candidates = g_slist_prepend(final_candidates, trcand);

		if (cand->type == NICE_CANDIDATE_TYPE_PEER_REFLEXIVE &&
						cand->priority <= priority)
			continue;

		priority = cand->priority;
		nice_address_to_string(&(cand->addr), addr);
		port = nice_address_get_port(&(cand->addr));
	}

	g_slist_free_full(candidates, (GDestroyNotify) nice_candidate_free);

	for (i = 0; i < self->priv->local_spec->medias->len; i++) {
		KmsMediaSpec *media;

		media = self->priv->local_spec->medias->pdata[i];

		if (!g_hash_table_lookup(media->type, (gpointer) type)) {
			continue;
		}

		media->transport->__isset_ice = TRUE;

		for (l = final_candidates; l != NULL; l = l->next)
			g_ptr_array_add(media->transport->ice->candidates, l->data);

		if (priority != 0) {
			media->transport->__isset_rtp = TRUE;
			media->transport->rtp->__isset_port = TRUE;
			media->transport->rtp->port = port;
			if (media->transport->rtp->address != NULL)
				g_free(media->transport->rtp->address);
			media->transport->rtp->__isset_address = TRUE;
			media->transport->rtp->address = g_strdup(addr);
		}
	}

	g_slist_free(final_candidates);
}

static void
constructed(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->constructed(object);

	if (kms_get_pipeline() == NULL) {
		g_warning("Kms should be initialized before instantiate "
								"objects");
		g_assert_not_reached();
		return;
	}

	create_nice_agents(self);
	process_candidates(self, self->priv->audio_agent, KMS_MEDIA_TYPE_AUDIO);
	process_candidates(self, self->priv->video_agent, KMS_MEDIA_TYPE_VIDEO);

	g_return_if_fail(self->priv->local_spec != NULL);

	self->priv->receiver = g_object_new(KMS_TYPE_RTP_RECEIVER, NULL);
	self->priv->sender = g_object_new(KMS_TYPE_RTP_SENDER, NULL);
}

static void
kms_rtp_connection_dispose(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	/* TODO: Unset manager on endpoint */

	LOCK(self);
	dispose_local_spec(self);
	dispose_remote_spec(self);
	dispose_neg_local_spec(self);
	dispose_neg_remote_spec(self);
	dispose_descriptor(self);
	dispose_receiver(self);
	dispose_sender(self);
	dispose_audio_agent(self);
	dispose_video_agent(self);
	UNLOCK(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->dispose(object);
}

static void
kms_rtp_connection_finalize(GObject *object) {
	KmsRtpConnection *self = KMS_RTP_CONNECTION(object);

	g_mutex_clear(&(self->priv->mutex));
	g_mutex_clear(&(self->priv->ice_mutex));
	g_cond_clear(&(self->priv->ice_cond));

	/* Chain up to the parent class */
	G_OBJECT_CLASS(kms_rtp_connection_parent_class)->finalize(object);
}

static void
kms_rtp_connection_class_init(KmsRtpConnectionClass *klass) {
	GParamSpec *pspec;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private (klass, sizeof (KmsRtpConnectionPriv));

	KMS_CONNECTION_CLASS(klass)->mode_changed = mode_changed;
	KMS_CONNECTION_CLASS(klass)->connect_to_remote = connect_to_remote;
	gobject_class->dispose = kms_rtp_connection_dispose;
	gobject_class->finalize = kms_rtp_connection_finalize;
	gobject_class->constructed = constructed;
	gobject_class->set_property = set_property;
	gobject_class->get_property = get_property;

	pspec = g_param_spec_object("local-spec", "Local Session Spec",
					"Local Session Spec",
					KMS_TYPE_SESSION_SPEC,
					G_PARAM_CONSTRUCT_ONLY |
					G_PARAM_READWRITE);

	g_object_class_install_property(gobject_class, PROP_LOCAL_SPEC, pspec);

	pspec = g_param_spec_object("remote-spec", "Remote Session Spec",
					"Remote Session Spec",
					KMS_TYPE_SESSION_SPEC,
					G_PARAM_READABLE);

	g_object_class_install_property(gobject_class, PROP_REMOTE_SPEC, pspec);

	pspec = g_param_spec_object("descriptor", "Session descriptor",
					"The current session descriptor",
					KMS_TYPE_SESSION_SPEC,
					G_PARAM_READABLE);

	g_object_class_install_property(gobject_class, PROP_DESCRIPTOR, pspec);
}

static void
kms_rtp_connection_init(KmsRtpConnection *self) {
	self->priv = KMS_RTP_CONNECTION_GET_PRIVATE(self);

	g_mutex_init(&(self->priv->mutex));
	self->priv->local_spec = NULL;
	self->priv->descriptor = NULL;
	self->priv->receiver = NULL;
	self->priv->sender = NULL;
	self->priv->remote_spec = NULL;
	self->priv->neg_remote_spec = NULL;
	self->priv->neg_local_spec = NULL;
	self->priv->audio_agent = NULL;
	self->priv->video_agent = NULL;
	self->priv->audio_gathered = FALSE;
	self->priv->video_gathered = FALSE;
	self->priv->audio_connected = FALSE;
	self->priv->video_connected = FALSE;
	g_mutex_init(&(self->priv->ice_mutex));
	g_cond_init(&(self->priv->ice_cond));
}
