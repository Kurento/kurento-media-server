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

#include <config.h>
#include "version.hpp"
#include <boost/concept_check.hpp>

#include <iostream>

const char *
get_version ()
{
  return PROJECT_VERSION;
}

void
print_version ()
{
  std::cout << "Version: " << PROJECT_VERSION << std::endl;
  std::cout << "TODO: print modules versions" << std::endl;
}
