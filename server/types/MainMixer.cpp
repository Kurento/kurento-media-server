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

#include "MainMixer.hpp"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_main_mixer
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMainMixer"

#define FACTORY_NAME "mainmixer"
#define MAIN_END_POINT "main"
#define MAIN_PORT_DEFAULT (-1)

namespace kurento
{
/* default constructor */
MainMixer::MainMixer (MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
                      const std::map<std::string, KmsMediaParam> &params)
  : Mixer (mediaSet, parent, g_KmsMediaMainMixerType_constants.TYPE_NAME,
           params, FACTORY_NAME)
{

}

MainMixer::~MainMixer() throw ()
{
}

void
MainMixer::setMainMixer (std::shared_ptr<MixerEndPoint> &mixerEndPoint)
{
  int id = mixerEndPoint->getHandlerId();
  g_object_set (G_OBJECT (element), MAIN_END_POINT, id, NULL);
}

void
MainMixer::unsetMainMixer()
{
  g_object_set (G_OBJECT (element), MAIN_END_POINT, MAIN_PORT_DEFAULT, NULL);
}

void
MainMixer::invoke (KmsMediaInvocationReturn &_return,
                   const std::string &command,
                   const std::map< std::string, KmsMediaParam > &params)
throw (KmsMediaServerException)
{
  if (g_KmsMediaMainMixerType_constants.SET_MAIN_END_POINT.compare (
        command) == 0) {
    const KmsMediaParam *p;
    std::shared_ptr<MixerEndPoint> mep;
    KmsMediaObjectRef mixerEndPoint;

    p = getParam (params,
                  g_KmsMediaMainMixerType_constants.SET_MAIN_END_POINT_PARAM_MIXER);

    if (p != NULL) {
      unmarshalStruct (mixerEndPoint, p->data);
      mep = getMediaSet().getMediaObject<MixerEndPoint> (mixerEndPoint);
      setMainMixer (mep);
    }
  } else if (g_KmsMediaMainMixerType_constants.UNSET_MAIN_END_POINT.compare (
               command) == 0) {
    unsetMainMixer ();
  } else {
    Mixer::invoke (_return, command, params);
  }
}

MainMixer::StaticConstructor MainMixer::staticConstructor;

MainMixer::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
