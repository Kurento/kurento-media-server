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

#include "JackVaderFilter.hpp"

#include "KmsMediaJackVaderFilterType_constants.h"

#include "utils/utils.hpp"
#include "KmsMediaErrorCodes_constants.h"

#define GST_CAT_DEFAULT kurento_jack_vader_filter
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoJackVaderFilter"

namespace kurento
{

void
JackVaderFilter::init (std::shared_ptr<MediaPipeline> parent)
{
  g_object_set (element, "filter-factory", "jackvader", NULL);
}

JackVaderFilter::JackVaderFilter (MediaSet &mediaSet,
                                  std::shared_ptr<MediaPipeline> parent,
                                  const std::map<std::string, KmsMediaParam> &params)
  : Filter (mediaSet, parent, g_KmsMediaJackVaderFilterType_constants.TYPE_NAME,
            params)
{
  init (parent);
}

JackVaderFilter::~JackVaderFilter() throw ()
{
}

JackVaderFilter::StaticConstructor JackVaderFilter::staticConstructor;

JackVaderFilter::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} // kurento
