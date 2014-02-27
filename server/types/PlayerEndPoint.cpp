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
#include "utils/marshalling.hpp"
#include "utils/utils.hpp"

#define GST_CAT_DEFAULT kurento_player_end_point
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoPlayerEndPoint"

#define FACTORY_NAME "playerendpoint"
#define USE_ENCODED_MEDIA "use-encoded-media"

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

PlayerEndPoint::PlayerEndPoint (MediaSet &mediaSet,
                                std::shared_ptr<MediaPipeline> parent,
                                const std::map<std::string, KmsMediaParam> &params)
throw (KmsMediaServerException)
  : UriEndPoint (mediaSet, parent,
                 g_KmsMediaPlayerEndPointType_constants.TYPE_NAME, params, FACTORY_NAME)
{
  const KmsMediaParam *p;
  KmsMediaPlayerEndPointConstructorParams constructorParams;

  p = getParam (params,
                g_KmsMediaPlayerEndPointType_constants.CONSTRUCTOR_PARAMS_DATA_TYPE);

  if (p != NULL) {
    unmarshalStruct (constructorParams, p->data);

    if (constructorParams.__isset.useEncodedMedia) {
      g_object_set (G_OBJECT (element), USE_ENCODED_MEDIA,
                    constructorParams.useEncodedMedia, NULL);
    }
  }

  g_signal_connect (element, "eos", G_CALLBACK (player_eos), this);
  g_signal_connect (element, "invalid-uri", G_CALLBACK (player_invalid_uri),
                    this);
  g_signal_connect (element, "invalid-media", G_CALLBACK (player_invalid_media),
                    this);
}

PlayerEndPoint::~PlayerEndPoint() throw ()
{
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
