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

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "KmsMediaServer_types.h"

namespace kurento
{

void generateUUID (std::string &_return);

void createKmsMediaServerException (KmsMediaServerException &_return, const std::string &description) throw (KmsMediaServerException);
void createKmsMediaServerException (KmsMediaServerException &_return, int32_t errorCode, const std::string &description) throw (KmsMediaServerException);

int32_t getI32Param (const std::map<std::string, KmsMediaParam>& params, const std::string &paramName);

void getStringParam (std::string &_return, const std::map<std::string, KmsMediaParam>& params, const std::string &paramName);
// TODO: Remove this method or make it inline
void setStringParam (std::map<std::string, KmsMediaParam>& params, const std::string &paramName, const std::string &paramValue);

const KmsMediaParam *getParam (const std::map<std::string, KmsMediaParam>& params, const std::string &paramName);

} // kurento

#endif /* __UTILS_HPP__ */
