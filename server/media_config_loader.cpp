#include "media_config_loader.h"
#include "log.h"

using ::com::kurento::log::Log;

static Log l("media_config_loader");
#define d(...) aux_debug(l, __VA_ARGS__);
#define i(...) aux_info(l, __VA_ARGS__);
#define e(...) aux_error(l, __VA_ARGS__);
#define w(...) aux_warn(l, __VA_ARGS__);

#define DEFAULT_ID "12345"

#define MEDIAS_KEY "medias"

#define MEDIA_GROUP "Media"
#define CODECS_KEY "codecs"
#define TRANSPORT_KEY "transport"

#define TRANSPORT_GROUP "Transport"

#define CODEC_GROUP "Codec"
#define ID_KEY "id"
#define NAME_KEY "name"
#define CLOCKRATE_KEY "clockRate"
#define CHANNELS "channels"
#define WIDTH "width"
#define HEIGHT "height"
#define BITRATE "bitrate"

#define EXTRA_PARAMS "extra"

using ::com::kurento::commons::mediaspec::MediaSpec;
using ::com::kurento::commons::mediaspec::Transport;
using ::com::kurento::commons::mediaspec::Payload;

static void
load_codec(Glib::KeyFile &configFile, const std::string &codecgrp, Payload &pay) {
	// TODO: Implement this function
	d("Loading config for: " + codecgrp);
	throw Glib::KeyFileError(Glib::KeyFileError::NOT_FOUND, "Not implemented");
}

static void
load_transport(Glib::KeyFile &configFile, const std::string &transportgrp,
								Transport &tr) {
	// TODO: Implement this function
	d("Loading config for: " + transportgrp);
	throw Glib::KeyFileError(Glib::KeyFileError::NOT_FOUND, "Not implemented");
}

static void
load_media(Glib::KeyFile &configFile, const std::string &mediagrp,
							MediaSpec &media)
{
	d("Loading config for media: " + mediagrp);
	if (!configFile.has_group(mediagrp)) {
		e("No codecs set, you won't be able to communicate with others");
		return;
	}

	Glib::ArrayHandle<Glib::ustring> codecs =
				configFile.get_string_list(mediagrp, CODECS_KEY);

	Glib::ArrayHandle<Glib::ustring>::const_iterator it = codecs.begin();
	for (; it != codecs.end(); it ++) {
		Payload pay;
		try {
			load_codec(configFile, CODEC_GROUP " " + *it, pay);
			media.payloads.push_back(pay);
		} catch (Glib::KeyFileError err) {
			w(err.what());
			w("error loading codec configuration: " + *it);
		}
	}

	std::string transport = configFile.get_string(mediagrp, TRANSPORT_KEY);
	load_transport(configFile, TRANSPORT_GROUP " " + transport,
							media.transport);
}

void
load_spec(Glib::KeyFile &configFile, SessionSpec &spec) {
	Glib::ArrayHandle<Glib::ustring> medias =
			configFile.get_string_list(SERVER_GROUP, MEDIAS_KEY);

	Glib::ArrayHandle<Glib::ustring>::const_iterator it = medias.begin();
	for (; it != medias.end(); it ++) {
		MediaSpec media;
		try {
			load_media(configFile, MEDIA_GROUP " " + *it, media);
			spec.medias.push_back(media);
		} catch (Glib::KeyFileError err) {
			w(err.what());
			w("Error loading media configuration: " + *it);
		}
	}

	spec.__set_id(DEFAULT_ID);
}
