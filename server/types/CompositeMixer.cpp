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

#include "CompositeMixer.hpp"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"
#include "KmsMediaCompositeMixerType_constants.h"

#define GST_CAT_DEFAULT kurento_compositer_mixer
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoCompositerMixer"

#define FACTORY_NAME "compositemixer"

namespace kurento
{
/* default constructor */
CompositeMixer::CompositeMixer (MediaSet &mediaSet,
                                std::shared_ptr<MediaPipeline> parent,
                                const std::map<std::string, KmsMediaParam> &params)
  : Mixer (mediaSet, parent, g_KmsMediaCompositeMixerType_constants.TYPE_NAME,
           params, FACTORY_NAME)
{

}

CompositeMixer::~CompositeMixer() throw ()
{
}

void
CompositeMixer::invoke (KmsMediaInvocationReturn &_return,
                        const std::string &command,
                        const std::map< std::string, KmsMediaParam > &params)
throw (KmsMediaServerException)
{
  Mixer::invoke (_return, command, params);
}

CompositeMixer::StaticConstructor CompositeMixer::staticConstructor;

CompositeMixer::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
