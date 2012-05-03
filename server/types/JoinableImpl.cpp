#include "types/JoinableImpl.h"

#include <glibmm.h>

#include <utils.h>

#include <log.h>

using ::com::kurento::kms::MediaObjectImpl;
using ::com::kurento::kms::JoinableImpl;
using ::com::kurento::kms::api::MediaSession;
using ::com::kurento::kms::api::Joinable;
using ::com::kurento::kms::api::MediaServerException;
using ::com::kurento::kms::api::ErrorCode;
using ::com::kurento::kms::api::JoinException;
using ::com::kurento::kms::api::StreamNotFoundException;
using ::com::kurento::kms::utils::get_media_type_from_stream;
using ::com::kurento::kms::utils::get_connection_mode_from_direction;
using ::com::kurento::kms::utils::get_inverse_connection_mode;

using ::com::kurento::log::Log;

static Log l("JoinableImpl");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

JoinableImpl::JoinableImpl(MediaSession &session) :
					MediaObjectImpl(session.object.token),
					Joinable() {
	__set_object(*this);
	__set_session(session);
}

JoinableImpl::~JoinableImpl() throw () {
	std::map<JoinableImpl *, KmsLocalConnection *>::iterator it;
	for (it = joinees.begin(); it != joinees.end(); it++) {
		g_object_unref(it->second);
	}
	joinees.clear();

	if (endpoint != NULL) {
		kms_endpoint_delete_all_connections(endpoint);
		g_object_unref(endpoint);
		endpoint = NULL;
	}
}

void
JoinableImpl::getStreams(std::vector<StreamType::type> &_return) {
	/* TODO: FIXME: By now all joinables has AUDIO and VIDEO by default.
	 * In the future this has to be get from endpoints in kmsclib */
	_return.push_back(StreamType::AUDIO);
	_return.push_back(StreamType::VIDEO);
}

void
JoinableImpl::join(const JoinableImpl& to, const Direction direction) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

KmsConnection*
JoinableImpl::create_local_connection() {
	GError *err = NULL;
	KmsConnection *lc;
	lc = kms_endpoint_create_connection(endpoint,
						KMS_CONNECTION_TYPE_LOCAL,
						&err);
	if (lc == NULL) {
		JoinException ex;
		if (err != NULL) {
			ex.__set_description(err->message);
			g_error_free(err);
		} else {
			ex.__set_description("Cannot create a local connection");
		}
		throw ex;
	}

	return lc;
}

void
JoinableImpl::join(JoinableImpl& to, const StreamType::type stream,
						const Direction direction) {
	if (!KMS_IS_ENDPOINT(endpoint)) {
		JoinException ex;
		ex.__set_description("Joinable does not have a valid endpoint");
		w(ex.description);
		throw ex;
	}

	std::map<JoinableImpl *, KmsLocalConnection *>::iterator it1;
	std::map<JoinableImpl *, KmsLocalConnection *>::iterator it2;
	KmsConnection *lc1, *lc2;

	it1 = joinees.find(&to);
	if (it1 == joinees.end()) {
		lc1 = create_local_connection();
		joinees[&to] = KMS_LOCAL_CONNECTION(lc1);
	} else {
		lc1 = KMS_CONNECTION(joinees[&to]);
	}

	it2 = to.joinees.find(this);
	if (it1 == joinees.end()) {
		lc2 = to.create_local_connection();
		to.joinees[this] = KMS_LOCAL_CONNECTION(lc2);
	} else {
		lc2 = KMS_CONNECTION(to.joinees[this]);
	}

	GError *err = NULL;
	KmsMediaType type;

	try {
		type = get_media_type_from_stream(stream);
	} catch (int ie) {
		StreamNotFoundException ex;
		ex.__set_description("Invalid stream");
		w(ex.description);
		throw ex;
	}

	if (!kms_connection_connect(lc1, lc2, type, &err)) {
		JoinException ex;
		if (err != NULL) {
			ex.__set_description(err->message);
			g_error_free(err);
		} else {
			ex.__set_description("Error linking local connections");
		}
		w(ex.description);
		throw ex;
	}

	KmsConnectionMode mode;
	KmsConnectionMode inverse_mode;

	try {
		mode = get_connection_mode_from_direction(direction);
		inverse_mode = get_inverse_connection_mode(mode);
	} catch (int ie) {
		JoinException ex;
		ex.__set_description("Bad direction");
		w(ex.description);
		throw ex;
	}

	if (!kms_connection_set_mode(lc1, mode, type, &err)) {
		JoinException ex;
		if (err != NULL) {
			ex.__set_description(err->message);
			g_error_free(err);
		} else {
			ex.__set_description("Cannot set mode");
		}
		w(ex.description);
		throw ex;
	}

	if (!kms_connection_set_mode(lc2, inverse_mode, type, &err)) {
		JoinException ex;
		if (err != NULL) {
			ex.__set_description(err->message);
			g_error_free(err);
		} else {
			ex.__set_description("Cannot set mode");
		}
		w(ex.description);
		throw ex;
	}
}

void
JoinableImpl::unjoin(JoinableImpl& to) {
	if (!KMS_IS_ENDPOINT(endpoint) && !KMS_IS_ENDPOINT(to.endpoint)) {
		JoinException ex;
		ex.__set_description("Joinable does not have a valid endpoint");
		w(ex.description);
		throw ex;
	}

	std::map<JoinableImpl *, KmsLocalConnection *>::iterator it;

	GError *err = NULL;

	it = joinees.find(&to);
	if (it != joinees.end()) {

		if (!kms_endpoint_delete_connection(endpoint,
						KMS_CONNECTION(it->second),
						&err)) {
			JoinException ex;
			if (err != NULL) {
				ex.__set_description(err->message);
				g_error_free(err);
			} else {
				ex.__set_description("Cannot set mode");
			}
			w(ex.description);
			throw ex;
		}
		g_object_unref(it->second);
		joinees.erase(it);
	}

	it = to.joinees.find(this);
	if (it == to.joinees.end()) {
		if (!kms_endpoint_delete_connection(to.endpoint,
						KMS_CONNECTION(it->second),
						&err)) {
			JoinException ex;
			if (err != NULL) {
				ex.__set_description(err->message);
				g_error_free(err);
			} else {
				ex.__set_description("Cannot set mode");
			}
			w(ex.description);
			throw ex;
		}
		g_object_unref(it->second);
		to.joinees.erase(it);
	}
}

void
JoinableImpl::unjoin(JoinableImpl& to, const StreamType::type stream) {
	if (!KMS_IS_ENDPOINT(endpoint) && !KMS_IS_ENDPOINT(to.endpoint)) {
		JoinException ex;
		ex.__set_description("Joinable does not have a valid endpoint");
		w(ex.description);
		throw ex;
	}

	std::map<JoinableImpl *, KmsLocalConnection *>::iterator it;

	GError *err = NULL;
	KmsMediaType type;

	try {
		type = get_media_type_from_stream(stream);
	} catch (int ie) {
		StreamNotFoundException ex;
		ex.__set_description("Invalid stream");
		w(ex.description);
		throw ex;
	}

	it = joinees.find(&to);
	if (it != joinees.end()) {
		if (!kms_connection_set_mode(KMS_CONNECTION(it->second),
				KMS_CONNECTION_MODE_INACTIVE, type, &err)) {
			JoinException ex;
			if (err != NULL) {
				ex.__set_description(err->message);
				g_error_free(err);
			} else {
				ex.__set_description("Cannot set mode");
			}
			w(ex.description);
			throw ex;
		}
	}

	it = to.joinees.find(this);
	if (it == to.joinees.end()) {
		if (!kms_connection_set_mode(KMS_CONNECTION(it->second),
				KMS_CONNECTION_MODE_INACTIVE, type, &err)) {
			JoinException ex;
			if (err != NULL) {
				ex.__set_description(err->message);
				g_error_free(err);
			} else {
				ex.__set_description("Cannot set mode");
			}
			w(ex.description);
			throw ex;
		}
	}
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return,
						const Direction direction) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return,
						const StreamType::type stream) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}

void
JoinableImpl::getJoinees(std::vector<Joinable>& _return,
						const StreamType::type stream,
						const Direction direction) {
	MediaServerException ex;
	ex.__set_description("Not implemented");
	ex.__set_code(ErrorCode::UNEXPECTED);
	throw ex;
}
