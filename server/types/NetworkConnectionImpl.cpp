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

#include "types/NetworkConnectionImpl.h"
#include <rtp/kms-rtp.h>
#include <rtmp/kms-rtmp-endpoint.h>

#include <utils.h>

#include <log.h>

using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
using ::com::kurento::kms::api::NetworkConnection;

using ::com::kurento::commons::mediaspec::MediaSpec;

using ::com::kurento::kms::utils::convert_session_spec;
using ::com::kurento::kms::utils::convert_session_spec_to_cpp;

using ::com::kurento::log::Log;

static Log l("NetworkConnectionImpl");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

NetworkConnectionImpl::NetworkConnectionImpl(MediaSession &session,
			const std::vector<NetworkConnectionConfig::type> & config,
			const SessionSpec &spec) :
				JoinableImpl(session), NetworkConnection(),
				spec(spec) {
	__set_joinable(*this);
	__set_config(config);

	KmsSessionSpec *local_spec;
	local_spec = convert_session_spec(this->spec);

	std::vector<NetworkConnectionConfig::type>::const_iterator it = config.begin();
	for (; it != config.end(); it++) {
		try {
			initialize_config(*it, local_spec);
		} catch (MediaServerException ex) {
			std::map <int, const char* > values =
				_NetworkConnectionConfig_VALUES_TO_NAMES;
			std::map <int, const char* >::const_iterator it =
								values.begin();

			for (; it != values.end(); it++) {
				finalize_config(
					(NetworkConnectionConfig::type) it->first);
			}

			g_object_unref(local_spec);
			throw ex;
		}
	}

	g_object_unref(local_spec);

	if (config.size() == 1) {
		endpoint = KMS_ENDPOINT(g_object_ref(endpoints.begin()->second));
		rtp_connection = KMS_CONNECTION(connections.begin()->second);
	}
}

NetworkConnectionImpl::~NetworkConnectionImpl() throw() {
	std::map <int, const char* > values = _NetworkConnectionConfig_VALUES_TO_NAMES;
	std::map <int, const char* >::const_iterator it = values.begin();

	rtp_connection = NULL;

	for (; it != values.end(); it++) {
		finalize_config((NetworkConnectionConfig::type) it->first);
	}
}

void
NetworkConnectionImpl::finalize_config(NetworkConnectionConfig::type config) {
	KmsEndpoint *ep;

	ep = endpoints[config];

	if (ep == NULL)
		return;

	GError *error = NULL;

	KmsConnection *connection = connections[config];

	if (connection != NULL) {
		if (!kms_endpoint_delete_connection(ep, connection, &error)) {
			if (error != NULL) {
				w("Error deleting rtp_connection: %s",
								error->message);
				g_error_free(error);
				error = NULL;
			} else {
				w("Unknown error deleting rtp_connection");
			}
			}
			g_object_unref(connection);
			connections[config] = NULL;
	}

	g_object_unref(ep);
	endpoints[config] = NULL;
}

void
NetworkConnectionImpl::initialize_config(
					NetworkConnectionConfig::type config,
					KmsSessionSpec *local_spec)
						throw (MediaServerException) {
	KmsEndpoint *ep;
	KmsConnection *connection;

	if (config == NetworkConnectionConfig::type::RTP)
		ep = KMS_ENDPOINT(g_object_new(KMS_TYPE_RTP_ENDPOINT,
							"local-spec",
							local_spec, NULL));
	else if (config == NetworkConnectionConfig::type::RTMP)
		ep= KMS_ENDPOINT(g_object_new(KMS_TYPE_RTMP_ENDPOINT,
							"local-spec",
							local_spec, NULL));

	if (ep == NULL) {
		MediaServerException ex;
		ex.__set_code(ErrorCode::NO_RESOURCES);
		ex.__set_description("Unable to create endpoint");
		w(ex.description);
		throw ex;
	}

	GError *error = NULL;

	connection =  kms_endpoint_create_connection(ep,
							KMS_CONNECTION_TYPE_RTP,
							&error);

	if (connection == NULL) {
		MediaServerException ex;

		g_object_unref(ep);
		ep = NULL;
		ex.__set_code(ErrorCode::NO_RESOURCES);
		if (error != NULL) {
			ex.__set_description(error->message);
			g_error_free(error);
			error = NULL;
		} else {
			ex.__set_description("Rtp connection cannnot be created");
		}
		w(ex.description);
		throw ex;
	}

	connections[config] = connection;
	endpoints[config] = ep;
}

void
NetworkConnectionImpl::generateOffer(SessionSpec& _return) {
	getLocalDescriptor(_return);
}

void
NetworkConnectionImpl::processAnswer(SessionSpec &_return,
						const SessionSpec& answer) {
	if (rtp_connection == NULL) {
		MediaServerException ex;
		ex.__set_description("RtpConnection is NULL");
		ex.__set_code(ErrorCode::UNEXPECTED);
		w(ex.description);
		throw ex;
	}

	KmsSessionSpec *spec = convert_session_spec(answer);
	GError *error = NULL;

	if (!kms_connection_connect_to_remote(rtp_connection, spec, TRUE,
								&error)) {
		MediaServerException ex;
		if (error != NULL) {
			ex.description = error->message;
			g_error_free(error);
		} else {
			ex.description = "Cannot negotiate format";
		}
		g_object_unref(spec);
		w(ex.description);
		throw ex;
	}
	g_object_unref(spec);

	getLocalDescriptor(_return);
}

void
NetworkConnectionImpl::processOffer(SessionSpec& _return,
						const SessionSpec& offer) {
	if (rtp_connection == NULL) {
		MediaServerException ex;
		ex.__set_description("RtpConnection is NULL");
		ex.__set_code(ErrorCode::UNEXPECTED);
		w(ex.description);
		throw ex;
	}

	KmsSessionSpec *spec = convert_session_spec(offer);
	GError *error = NULL;

	if (!kms_connection_connect_to_remote(rtp_connection, spec, FALSE,
								&error)) {
		MediaServerException ex;
		if (error != NULL) {
			ex.description = error->message;
			g_error_free(error);
		} else {
			ex.description = "Cannot negotiate format";
		}
		g_object_unref(spec);
		w(ex.description);
		throw ex;
	}
	g_object_unref(spec);

	getLocalDescriptor(_return);
}

static bool
getLocalDescriptorAux(SessionSpec& _return, KmsConnection *connection) {
	if (connection == NULL)
		return false;

	KmsSessionSpec *cspec;
	g_object_get(connection, "descriptor", &cspec, NULL);
	try {
		convert_session_spec_to_cpp(_return, cspec);
	} catch (MediaServerException ex) {
		g_object_unref(cspec);
		return false;
	}
	g_object_unref(cspec);

	return true;
}

static void
mergeSpecs(SessionSpec& from, SessionSpec& to) {
	std::vector<MediaSpec>::const_iterator it = from.medias.begin();

	for (; it != from.medias.end(); it++) {
		to.medias.push_back(*it);
	}

	to.id = from.id;
	to.version = from.version;
}

void
NetworkConnectionImpl::getLocalDescriptor(SessionSpec& _return) {
	if (rtp_connection != NULL) {
		getLocalDescriptorAux(_return, rtp_connection);
	} else {
		SessionSpec spec;
		bool found = false;

		std::map<NetworkConnectionConfig::type, KmsConnection *>::const_iterator it = connections.begin();
		for (; it != connections.end(); it++) {
			if (getLocalDescriptorAux(spec, it->second)) {
				mergeSpecs(spec, _return);
				found = true;
			}
		}

		if (!found) {
			MediaServerException ex;
			ex.__set_description("No valid descriptor found");
			ex.__set_code(ErrorCode::UNEXPECTED);
			w(ex.description);
			throw ex;
		}
	}
}

void
NetworkConnectionImpl::getRemoteDescriptor(SessionSpec& _return) {
	if (rtp_connection == NULL) {
		MediaServerException ex;
		ex.__set_description("RtpConnection is NULL");
		ex.__set_code(ErrorCode::UNEXPECTED);
		w(ex.description);
		throw ex;
	}

	KmsSessionSpec *cspec;
	g_object_get(rtp_connection, "remote-spec", &cspec, NULL);
	if (cspec == NULL) {
		NegotiationException ex;
		ex.__set_description("Not negotiated yet");
		throw ex;
	}

	try {
		convert_session_spec_to_cpp(_return, cspec);
	} catch (MediaServerException ex) {
		g_object_unref(cspec);
		throw ex;
	}
	g_object_unref(cspec);
}
