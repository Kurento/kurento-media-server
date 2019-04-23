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

#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

// rlim_t: Unsigned integer type used for limit values
// (see `man sys_resource.h`)
#include <sys/resource.h>

namespace kurento
{

static const float DEFAULT_RESOURCE_LIMIT_PERCENT = 0.80f;

rlim_t getMaxThreads ();
rlim_t getMaxOpenFiles ();

void checkResources (float limit_percent);

void killServerOnLowResources (float limit_percent);

} /* kurento */

#endif /* __RESOURCE_MANAGER_H__ */
