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

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <gst/gst.h>
#include <glibmm.h>
#include "ExponentialBackoffStrategy.hpp"

#define DEFAULT_LIMIT (30 * 1000) /* seconds */

#define GST_CAT_DEFAULT kurento_rabbitmq_exponential_backoff_strategy
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoExponentialBackoffStrategy"

namespace kurento
{

ExponentialBackoffStrategy::ExponentialBackoffStrategy (int max)
{
  if (limit > 0) {
    limit = max;
  } else {
    limit = DEFAULT_LIMIT;
  }

  /* initialize random seed */
  srand (time (NULL) );

  reset ();
}

void
ExponentialBackoffStrategy::reset ()
{
  attemp = 1;
}

int
ExponentialBackoffStrategy::getTimeout ()
{
  int timeout, max;

  max = (int) ( (pow (2.0, attemp++) - 1) * 1000);

  /* generate a random interval of time between 0 and 2^attemp - 1 */
  timeout = rand() % max;

  if (timeout > limit) {
    timeout = limit;
  }

  return timeout;
}

ExponentialBackoffStrategy::~ExponentialBackoffStrategy()
{
}

ExponentialBackoffStrategy::StaticConstructor
ExponentialBackoffStrategy::staticConstructor;

ExponentialBackoffStrategy::StaticConstructor::StaticConstructor()
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);
}

} /* kurento */



