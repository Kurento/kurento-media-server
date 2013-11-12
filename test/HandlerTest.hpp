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

#ifndef __HANDLER_TEST_HPP__
#define __HANDLER_TEST_HPP__

#include "KmsMediaHandlerService.h"
#include <glibmm/thread.h>
#include <functional>

#define HANDLER_IP "localhost"
#define HANDLER_PORT 9191
namespace kurento
{

class HandlerTest : virtual public KmsMediaHandlerServiceIf
{
public:
  HandlerTest();
  ~HandlerTest();

  void onEvent(const std::string& callbackToken, const KmsMediaEvent& event);
  void onError(const std::string& callbackToken, const KmsMediaError& error);
  void setEventFunction (const std::function
                              <void (std::string, KmsMediaEvent)> &function,
                         std::string waitEvent);
  void setErrorFunction (const std::function
                              <void (std::string, KmsMediaError)> &function,
                         std::string waitError);

private:
  Glib::RecMutex mutex;

  std::function <void (std::string, KmsMediaEvent)> eventFunc;
  std::function <void (std::string, KmsMediaError)> errorFunc;
  std::string waitEvent;
  std::string waitError;
};

} // kurento

#endif /* __HANDLER_TEST_HPP__ */
