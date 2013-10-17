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

#include "utils.hpp"

#include "marshalling.hpp"
#include "KmsMediaErrorCodes_constants.h"

#include <glibmm.h>
#include <uuid/uuid.h>

/* 36-byte string (plus tailing '\0') */
#define UUID_STR_SIZE 37

namespace kurento
{

void
generateUUID (std::string &_return)
{
  uuid_t uuid;
  char *uuid_str;

  uuid_str = (char *) g_malloc (UUID_STR_SIZE);
  uuid_generate (uuid);
  uuid_unparse (uuid, uuid_str);
  _return = uuid_str;
  g_free (uuid_str);
}

void
createKmsMediaServerException (KmsMediaServerException &_return,
                               const std::string &description) throw (KmsMediaServerException)
{
  _return.__set_errorCode (g_KmsMediaErrorCodes_constants.MEDIA_ERROR);
  _return.__set_description (description);
}

void
createKmsMediaServerException (KmsMediaServerException &_return,
                               int32_t errorCode,
                               const std::string &description) throw (KmsMediaServerException)
{
  _return.__set_errorCode (errorCode);
  _return.__set_description (description);
}

int32_t
getI32Param (const std::map<std::string, KmsMediaParam> &params, const std::string &paramName)
{
  std::map< std::string, KmsMediaParam>::const_iterator it;
  it = params.find (paramName);

  if (it != params.end () ) {
    return unmarshalI32Param (it->second);
  } else {
    KmsMediaServerException except;

    createKmsMediaServerException (except, g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR,
                                   "Param '" + paramName + "' not found");
    throw except;
  }
}

void
getStringParam (std::string &_return,
                const std::map<std::string, KmsMediaParam> &params,
                const std::string &paramName)
{
  std::map< std::string, KmsMediaParam>::const_iterator it;
  it = params.find (paramName);

  if (it != params.end () ) {
    unmarshalStringParam (_return, it->second);
  } else {
    KmsMediaServerException except;

    createKmsMediaServerException (except, g_KmsMediaErrorCodes_constants.MEDIA_OBJECT_ILLEGAL_PARAM_ERROR,
                                   "Param '" + paramName + "' not found");
    throw except;
  }
}


void
setStringParam (std::map<std::string, KmsMediaParam> &params, const std::string &paramName, const std::string &paramValue)
{
  createStringParam (params[paramName], paramValue);
}

const KmsMediaParam *
getParam (const std::map<std::string, KmsMediaParam> &params, const std::string &paramName)
{
  std::map< std::string, KmsMediaParam>::const_iterator it;
  it = params.find (paramName);

  if (it != params.end () ) {
    return & (it->second);
  }

  return NULL;
}

} // kurento
