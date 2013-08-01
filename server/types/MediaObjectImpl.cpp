/*
 * MediaObjectImpl.cpp - Kurento Media Server
 *
 * Copyright (C) 2013 Kurento
 * Contact: Miguel París Díaz <mparisdiaz@gmail.com>
 * Contact: José Antonio Santos Cadenas <santoscadenas@kurento.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MediaObjectImpl.hpp"

#include <glibmm.h>
#include <uuid/uuid.h>

namespace kurento
{

static ObjectId
getId()
{
  static Glib::Mutex mutex;
  static int seed_initiated = 0;
  int64_t ret;

  mutex.lock();

  if (!seed_initiated) {
    srand48 (time (NULL) );
    seed_initiated = 1;
  }

  ret = lrand48();
  mutex.unlock();

  return ret;
}

MediaObjectImpl::MediaObjectImpl() : MediaObject()
{
  uuid_t uuid;
  char *uuid_str;
  std::string tk;

  uuid_str = (char *) g_malloc (sizeof (char) * 37);
  uuid_generate (uuid);
  uuid_unparse (uuid, uuid_str);
  tk.append (uuid_str);
  g_free (uuid_str);

  id = getId();
  this->token = tk;
}

MediaObjectImpl::MediaObjectImpl (std::shared_ptr<MediaObject> parent) : MediaObject()
{
  id = getId();
  this->token = parent->token;
  this->parent = parent;
}

MediaObjectImpl::~MediaObjectImpl() throw ()
{
}

std::shared_ptr<MediaObject>
MediaObjectImpl::getParent () throw (NoParentException)
{
  if (parent == NULL) {
    throw NoParentException ();
  }

  return parent;
}

gchar *
MediaObjectImpl::getIdStr()
{
  return g_strdup_printf  ("%" G_GUINT64_FORMAT, id);
}

} // kurento
