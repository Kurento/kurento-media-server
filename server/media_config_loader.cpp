/*
 * media_config_loader.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "media_config_loader.hpp"
#include <gst/gst.h>
#include "log.hpp"

#define DEFAULT_VERSION "0"
#define DEFAULT_ID "12345"

#define MEDIAS_KEY "medias"

#define MEDIA_GROUP "Media"
#define CODECS_KEY "codecs"
#define TRANSPORT_KEY "transport"
#define DIRECTION_KEY "direction"
#define TYPE_KEY "type"

#define TRANSPORT_GROUP "Transport"
#define RTP_KEY "rtp"
#define RTMP_KEY "rtmp"

#define TRANSPORT_RTP_GROUP "TransportRtp"
#define ADDRESS_KEY "address"

#define TRANSPORT_RTMP_GROUP "TransportRtmp"
#define URL_KEY "url"

#define CODEC_GROUP "Codec"

#define CODEC_RTP_GROUP "CodecRtp"
#define ID_KEY "id"
#define NAME_KEY "name"
#define CLOCKRATE_KEY "clockRate"
#define CHANNELS_KEY "channels"
#define WIDTH_KEY "width"
#define HEIGHT_KEY "height"
#define BITRATE_KEY "bitrate"

#define EXTRA_PARAMS_KEY "extra"

static gchar *
gen_id ()
{
  static int curr_id = 96;
  return g_strdup_printf ("%i", curr_id++);
}

static void
load_codec_rtp (Glib::KeyFile &configFile, const std::string &codecgrp,
    GstSDPMedia *media)
{
  Glib::ustring rtpmap, fmtp;
  Glib::ustring codec_name, clock_rate, channels, extra_attrs,
       new_extra_attr;
  gchar *id;

  GST_DEBUG ("Loading config for: %s", codecgrp.c_str () );

  codec_name = configFile.get_string (codecgrp, NAME_KEY).uppercase ();
  clock_rate = configFile.get_string (codecgrp, CLOCKRATE_KEY);

  try {
    channels = configFile.get_string (codecgrp, CHANNELS_KEY);
  } catch (Glib::KeyFileError ex) {
    channels = "";
  }

  extra_attrs = "";

  try {
    Glib::ArrayHandle < Glib::ustring > extra =
      configFile.get_string_list (codecgrp, EXTRA_PARAMS_KEY);
    Glib::ArrayHandle < Glib::ustring >::const_iterator it = extra.begin ();

    for (; it != extra.end (); it++) {
      try {
        new_extra_attr = "";
        new_extra_attr.append (*it);
        new_extra_attr.append ("=");
        new_extra_attr.append (configFile.get_string (codecgrp, *it) );
      } catch (Glib::KeyFileError ex) {
        new_extra_attr = "";
      }

      if (extra_attrs != "")
        extra_attrs.append ("; ");

      extra_attrs = extra_attrs.append (new_extra_attr);
    }
  } catch (Glib::KeyFileError ex) {
  }

  id = gen_id ();

  rtpmap = "";
  rtpmap.append (id).append (" ").append (codec_name).append ("/").
  append (clock_rate);

  if (channels != "")
    rtpmap.append ("/").append (channels);

  fmtp = "";

  if (extra_attrs != "") {
    fmtp = id;
    fmtp.append (" ").append (extra_attrs);
  }

  gst_sdp_media_add_format (media, id );
  gst_sdp_media_add_attribute (media, "rtpmap", rtpmap.c_str () );

  if (fmtp != "")
    gst_sdp_media_add_attribute (media, "fmtp", fmtp.c_str () );

  g_free (id);
}

static void
load_codec (Glib::KeyFile &configFile, const std::string &codecgrp,
    GstSDPMedia *media)
{
  GST_DEBUG ("Loading config for: %s", codecgrp.c_str () );

  try {
    std::string rtp = configFile.get_string (codecgrp, RTP_KEY);
    load_codec_rtp (configFile, CODEC_RTP_GROUP " " + rtp, media);
  } catch (Glib::KeyFileError ex) {
    GST_WARNING (ex.what ().c_str () );
  }
}

static GstSDPMedia *
load_media (Glib::KeyFile &configFile, const std::string &mediagrp)
{
  GstSDPMedia *media = NULL;
  GST_DEBUG ("Loading config for media: %s", mediagrp.c_str () );

  if (!configFile.has_group (mediagrp) ) {
    GST_WARNING ("No codecs set, you won't be able to communicate with others");
    return NULL;
  }

  gst_sdp_media_new (&media);   // TODO: gst_sdp_media_free when needed
  gst_sdp_media_init (media);   // TODO: gst_sdp_media_free when needed
  Glib::ArrayHandle < Glib::ustring > codecs =
    configFile.get_string_list (mediagrp, CODECS_KEY);
  Glib::ArrayHandle < Glib::ustring >::const_iterator it = codecs.begin ();

  for (; it != codecs.end (); it++) {
    try {
      load_codec (configFile, CODEC_GROUP " " + *it, media);
    } catch (Glib::KeyFileError err) {
      GST_WARNING (err.what ().c_str () );
      GST_WARNING ("Error loading codec configuration");
    }
  }

  Glib::ustring type = configFile.get_string (mediagrp, TYPE_KEY);
  gst_sdp_media_set_media (media, type.c_str () );
  std::string transport = configFile.get_string (mediagrp, TRANSPORT_KEY);
  gst_sdp_media_set_proto (media, "RTP/AVP");
  return media;
}

GstSDPMessage *
load_session_descriptor (Glib::KeyFile &configFile)
{
  GstSDPMessage *sdp_message;
  GstSDPMedia *media;
  GstSDPResult result;
  GST_DEBUG ("Load Session Descriptor");
  sdp_message = NULL;
  gchar *sdp_message_text = NULL;
  result = gst_sdp_message_new (&sdp_message);

  if (result != GST_SDP_OK) {
    GST_ERROR ("Error creating sdp message");
    return NULL;
  }

  Glib::ArrayHandle < Glib::ustring > medias =
    configFile.get_string_list (SERVER_GROUP, MEDIAS_KEY);
  Glib::ArrayHandle < Glib::ustring >::const_iterator it = medias.begin ();

  for (; it != medias.end (); it++) {
    try {
      media = load_media (configFile, MEDIA_GROUP " " + *it);
      gst_sdp_message_add_media (sdp_message, media);
    } catch (Glib::KeyFileError err) {
      GST_WARNING (err.what ().c_str () );
      GST_WARNING ("Error loading media configuration");
    }
  }

  gst_sdp_message_set_version (sdp_message, DEFAULT_VERSION);
  gst_sdp_message_set_session_name (sdp_message, "-");
  gst_sdp_message_set_connection (sdp_message, "IN", "IP4", "localhost", 0, 0);
  gst_sdp_message_set_origin (sdp_message, "-", DEFAULT_ID, "0", "IN", "IP4",
      "localhost");

  GST_DEBUG ("SDP: \n%s", sdp_message_text = gst_sdp_message_as_text (sdp_message) );
  g_free (sdp_message_text);

  return sdp_message;
}
