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

/*
 * This file tries to solve a problem while linking modules that useful
 * libgnutls.so.28 and others that link libgnutls.so.26
 * The fix forces loading of libgnutls.so.28 that ensures that webrtc works
 * correctly
 */

#include <gnutls/gnutls.h>

void kms_dtls_load_fix();

void
kms_dtls_load_fix()
{
  gnutls_url_is_supported ("");
}
