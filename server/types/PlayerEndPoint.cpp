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

#include "KmsMediaUriEndPointType_constants.h"
#include "KmsMediaPlayerEndPointType_constants.h"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"

#define GST_CAT_DEFAULT kurento_player_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoPlayerEndPoint"

#define FACTORY_NAME "playerendpoint"

namespace kurento
{

void
player_eos (GstElement *player, PlayerEndPoint *self)
{
  self->sendEvent (g_KmsMediaPlayerEndPointType_constants.EVENT_EOS);
}

void
player_invalid_uri (GstElement *player, PlayerEndPoint *self)
{
  self->sendError ("invalid-uri", "invalid-uri",
                   g_KmsMediaErrorCodes_constants.CONNECT_ERROR);
}

void
player_invalid_media (GstElement *player, PlayerEndPoint *self)
{
  self->sendError ("invalid-media", "invalid-media",
                   g_KmsMediaErrorCodes_constants.MEDIA_ERROR);
}

void
PlayerEndPoint::init (std::shared_ptr<MediaPipeline> parent,
                      const std::string &uri)
{
  g_object_set (G_OBJECT (element), "uri", uri.c_str(), NULL);

  g_signal_connect (element, "eos", G_CALLBACK (player_eos), this);
  g_signal_connect (element, "invalid-uri", G_CALLBACK (player_invalid_uri),
                    this);
  g_signal_connect (element, "invalid-media", G_CALLBACK (player_invalid_media),
                    this);

  g_object_ref (element);
  gst_bin_add (GST_BIN (parent->pipeline), element);
  gst_element_sync_state_with_parent (element);
}

PlayerEndPoint::PlayerEndPoint (MediaSet &mediaSet,
                                std::shared_ptr<MediaPipeline> parent,
                                const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : UriEndPoint (mediaSet, parent,
                 g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params, FACTORY_NAME)
{
  const KmsMediaParam *p;
  KmsMediaUriEndPointConstructorParams uriEpParams;

  p = getParam (params,
                g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);

  if (p == NULL) {
    KmsMediaServerException except;

    createKmsMediaServerException (except,
                                   g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR,
                                   "Param '" + g_KmsMediaUriEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE +
                                   "' not found");
    throw except;
  }

  unmarshalKmsMediaUriEndPointConstructorParams (uriEpParams, p->data);

  init (parent, uriEpParams.uri);
}

PlayerEndPoint::~PlayerEndPoint() throw ()
{
  gst_bin_remove (GST_BIN ( std::dynamic_pointer_cast<MediaPipeline>
                            (parent)->pipeline), element);
  gst_element_set_state (element, GST_STATE_NULL);
  g_object_unref (element);
}

void
PlayerEndPoint::subscribe (std::string &_return, const std::string &eventType,
                           const std::string &handlerAddress, const int32_t handlerPort)
throw (KmsMediaServerException)
{
  if (g_KmsMediaPlayerEndPointType_constants.EVENT_EOS.compare (eventType) == 0) {
    mediaHandlerManager.addMediaHandler (_return, eventType, handlerAddress,
                                         handlerPort);
  } else {
    UriEndPoint::subscribe (_return, eventType, handlerAddress, handlerPort);
  }
}

PlayerEndPoint::StaticConstructor PlayerEndPoint::staticConstructor;

PlayerEndPoint::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
