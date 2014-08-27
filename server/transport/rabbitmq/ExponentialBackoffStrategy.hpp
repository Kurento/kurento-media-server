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

#ifndef __EXPONENTIAL_BACKOFF_STRATEGY_HPP__
#define __EXPONENTIAL_BACKOFF_STRATEGY_HPP__

#include "RabbitMQReconnectStrategy.hpp"

namespace kurento
{

class ExponentialBackoffStrategy: public RabbitMQReconnectStrategy
{
public:
  ExponentialBackoffStrategy (int max);
  virtual ~ExponentialBackoffStrategy();

  virtual int getTimeout();
  virtual void reset();

private:

  int attemp;
  int limit;

  class StaticConstructor
  {
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;
};

} /* kurento */

#endif /* __EXPONENTIAL_BACKOFF_STRATEGY_HPP__ */