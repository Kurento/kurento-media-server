/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __KURENTO_LOAD_CONFIG_HPP__
#define __KURENTO_LOAD_CONFIG_HPP__

#include <boost/property_tree/ptree.hpp>

namespace kurento
{

void
loadConfig (boost::property_tree::ptree &config, const std::string &file_name,
            const std::string &modulesConfigPath);

void
mergePropertyTrees (boost::property_tree::ptree &ptMerged,
                    const boost::property_tree::ptree &ptSecond, int level = 0 );

} /* kurento */

#endif /* __KURENTO_LOAD_CONFIG_HPP__ */
