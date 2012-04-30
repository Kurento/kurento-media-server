#include "types/NetworkConnectionImpl.h"
#include <rtp/kms-rtp.h>

#include <utils.h>

#include <log.h>

using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::NetworkConnectionImpl;
using ::com::kurento::kms::api::NetworkConnection;

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

	endpoint = KMS_ENDPOINT(g_object_new(KMS_TYPE_RTP_ENDPOINT, "local-spec",
							local_spec, NULL));
	g_object_unref(local_spec);

	if (endpoint == NULL) {
		MediaServerException ex;
		ex.__set_code(ErrorCode::NO_RESOURCES);
		ex.__set_description("Unable to create network connection");
		w(ex.description);
		throw ex;
	}

	GError *error = NULL;

	rtp_connection =  kms_endpoint_create_connection(endpoint,
							KMS_CONNECTION_TYPE_RTP,
							&error);

	if (rtp_connection == NULL) {
		MediaServerException ex;

		g_object_unref(endpoint);
		endpoint = NULL;
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
}

NetworkConnectionImpl::~NetworkConnectionImpl() throw() {
	if (endpoint == NULL)
		return;

	GError *error = NULL;

	if (rtp_connection != NULL) {
		if (!kms_endpoint_delete_connection(endpoint, rtp_connection,
								&error)) {
			if (error != NULL) {
				w("Error deleting rtp_connection: %s",
								error->message);
				g_error_free(error);
			} else {
				w("Unknown error deleting rtp_connection");
			}
		}
		g_object_unref(rtp_connection);
	}
	rtp_connection = NULL;
}

void
NetworkConnectionImpl::generateOffer(SessionSpec& _return) {
	getLocalDescriptor(_return);
}

void
NetworkConnectionImpl::processAnswer(SessionSpec &_return,
						const SessionSpec& anwser) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	w(ex.description);
	throw ex;
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

	_return = offer;
	getLocalDescriptor(_return);
}

void
NetworkConnectionImpl::getLocalDescriptor(SessionSpec& _return) {
	if (rtp_connection == NULL) {
		MediaServerException ex;
		ex.__set_description("RtpConnection is NULL");
		ex.__set_code(ErrorCode::UNEXPECTED);
		w(ex.description);
		throw ex;
	}

	KmsSessionSpec *cspec;
	g_object_get(rtp_connection, "descriptor", &cspec, NULL);
	try {
		convert_session_spec_to_cpp(_return, cspec);
	} catch (MediaServerException ex) {
		g_object_unref(cspec);
		throw ex;
	}
	g_object_unref(cspec);
}

void
NetworkConnectionImpl::getRemoteDescriptor(SessionSpec& _return) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	w(ex.description);
	throw ex;
}
