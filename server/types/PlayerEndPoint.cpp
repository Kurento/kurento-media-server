/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "PlayerEndPoint.hpp"

#include "KmsMediaPlayerEndPointType_constants.h"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#if 0
#include "protocol/TBinaryProtocol.h"
#include "transport/TBufferTransports.h"
#endif

#define GST_CAT_DEFAULT kurento_player_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoPlayerEndPoint"

#if 0
using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::protocol::TBinaryProtocol;
#endif

namespace kurento
{

static void
player_eos (GstElement *player, PlayerEndPoint *self)
{
// TODO: reuse when needed
#if 0
  MediaEvent event;
  EndOfStreamEvent eosEvent;
  PlayerEndPointEvent playerEvent;

  boost::shared_ptr<TMemoryBuffer> transport (new TMemoryBuffer() );
  TBinaryProtocol protocol (transport);

  playerEvent.__set_eos (eosEvent);
  playerEvent.write (&protocol);
  std::string event_str;
  transport->appendBufferToString (event_str);
  event.__set_event (event_str);
  event.__set_source (*self);

  std::dynamic_pointer_cast<MediaPipeline> (self->parent)->sendEvent (event);
  GST_DEBUG ("Player finished");
#endif
}

void
PlayerEndPoint::init (std::shared_ptr<MediaPipeline> parent, const std::string &uri)
{
  element = gst_element_factory_make ("playerendpoint", NULL);

  g_object_set (G_OBJECT (element), "uri", uri.c_str(), NULL);

  g_signal_connect (element, "eos", G_CALLBACK (player_eos), this);

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
}

PlayerEndPoint::PlayerEndPoint (std::shared_ptr<MediaPipeline> parent, const KmsMediaParams &params)
throw (KmsMediaServerException)
  : UriEndPoint (parent, g_KmsMediaPlayerEndPointType_constants.TYPE_NAME)
{
  if (g_KmsMediaDataType_constants.STRING_DATA_TYPE.compare (params.dataType) == 0) {
    std::string uri;
    uri = unmarshalString (params.data);
    init (parent, uri);
  } else {
    throw createKmsMediaServerException  (g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_CONSTRUCTOR_NOT_FOUND,
        "PlayerEndPoint has not any constructor with params of type " + params.dataType);
  }
}

PlayerEndPoint::~PlayerEndPoint() throw ()
{
  gst_bin_remove (GST_BIN ( ( (std::shared_ptr<MediaPipeline> &) parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

std::string
PlayerEndPoint::subscribe (const std::string &eventType, const std::string &handlerAddress, const int32_t handlerPort)
throw (KmsMediaServerException)
{
  if (g_KmsMediaPlayerEndPointType_constants.EVENT_EOS.compare (eventType) == 0) {
    return mediaHandlerManager.addMediaHandler (eventType, handlerAddress, handlerPort);
  }

  return UriEndPoint::subscribe (eventType, handlerAddress, handlerPort);
}

PlayerEndPoint::StaticConstructor PlayerEndPoint::staticConstructor;

PlayerEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);
}

} // kurento
