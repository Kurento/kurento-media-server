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

#include "CrowdDetectorFilter.hpp"

#include "KmsMediaCrowdDetectorFilterType_constants.h"

#include "utils/utils.hpp"
#include "utils/marshalling.hpp"
#include "KmsMediaDataType_constants.h"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_crowd_detector_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoCrowdDetectorFilter"

namespace kurento
{
/* default constructor */
CrowdDetectorFilter::CrowdDetectorFilter (
  MediaSet &mediaSet, std::shared_ptr<MediaPipeline> parent,
  const std::map<std::string, KmsMediaParam> &params)
  : Filter (mediaSet, parent,
            g_KmsMediaCrowdDetectorFilterType_constants.TYPE_NAME,
            params)
{
  g_object_set (element, "filter-factory", "crowdetector", NULL);

  GstElement *crowdDetector;

  g_object_get (G_OBJECT (element), "filter", &crowdDetector, NULL);

  this->crowdDetector = crowdDetector;
  g_object_unref (crowdDetector);
}

CrowdDetectorFilter::~CrowdDetectorFilter() throw ()
{
}

void
CrowdDetectorFilter::invoke (KmsMediaInvocationReturn &_return,
                             const std::string &command,
                             const std::map< std::string, KmsMediaParam > &params)
throw (KmsMediaServerException)
{
}

CrowdDetectorFilter::StaticConstructor CrowdDetectorFilter::staticConstructor;

CrowdDetectorFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
