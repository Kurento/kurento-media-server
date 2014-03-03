/*
 * (C) Copyright 2014 Kurento (http://kurento.org/)
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

#ifndef __MODULE_HPP__
#define __MODULE_HPP__

#include <memory>
#include <map>

#define KURENTO_MODULE_DESCRIPTOR "kurento_module_descriptor"

namespace kurento
{
class MediaPipeline;
class MediaSet;
class MediaObjectImpl;
}

extern "C" {

  enum _KurentoModuleType {
    KURENTO_MODULE_ELEMENT,
    KURENTO_MODULE_MIXER
  };

  typedef enum _KurentoModuleType KurentoModuleType;

  typedef std::shared_ptr<kurento::MediaObjectImpl> (*create_object_func) (
    kurento::MediaSet &mediaSet,
    std::shared_ptr< kurento::MediaPipeline > parent);

  typedef std::string (*get_name_func) ();

  typedef struct {
    KurentoModuleType type;
    get_name_func get_name;
    create_object_func create_object;
  } KurentoModule;

#define KURENTO_CREATE_MODULE(type, type_name, create_object_func)      \
  extern "C" {                                                          \
    KurentoModule kurento_module_descriptor =                           \
        { type, type_name, create_object_func };                        \
  }

}

#endif /* __MODULE_HPP__ */
