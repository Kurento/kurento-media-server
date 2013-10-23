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

#ifndef __MEDIA_OBJECT_PARENT_HPP__
#define __MEDIA_OBJECT_PARENT_HPP__

#include "MediaObjectImpl.hpp"
#include "common/MediaSet.hpp"

namespace kurento
{

class MediaObjectParent : public MediaObjectImpl
{
public:
  MediaObjectParent (MediaSet &mediaSet, const std::map<std::string, KmsMediaParam> &params = emptyParams);
  MediaObjectParent (MediaSet &mediaSet, std::shared_ptr<MediaObjectImpl> parent, const std::map<std::string, KmsMediaParam> &params = emptyParams);
  virtual ~MediaObjectParent() throw () = 0;

protected:

  void registerChild (std::shared_ptr<MediaObjectImpl> child);
  MediaSet &getMediaSet();

private:
  MediaSet &mediaSet;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} // kurento

#endif /* __MEDIA_OBJECT_PARENT_HPP__ */
