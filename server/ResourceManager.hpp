/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
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

#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

namespace kurento
{

static const float DEFAULT_RESOURCE_LIMIT_PERCENT = 0.8;

void checkResources (float limit_percent);

void killServerOnLowResources (float limit_percent);

} /* kurento */

#endif /* __RESOURCE_MANAGER_H__ */
