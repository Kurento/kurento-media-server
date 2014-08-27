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

#ifndef __RABBITMQ_RECONNECT_STRATEGY_HPP__
#define __RABBITMQ_RECONNECT_STRATEGY_HPP__

namespace kurento
{

class RabbitMQReconnectStrategy
{
public:
  virtual int getTimeout() = 0;
  virtual void reset() = 0;
};

} /* kurento */

#endif /* __RABBITMQ_RECONNECT_STRATEGY_HPP__ */