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

#include <config.h>

#include <glibmm.h>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include "version.hpp"
#include <glib/gstdio.h>
#include <ftw.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "services/Service.hpp"
#include "services/ServiceFactory.hpp"

#include <SignalHandler.hpp>
#include <gst/gst.h>

#define GST_CAT_DEFAULT kurento_media_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "KurentoMediaServer"

const std::string DEFAULT_CONFIG_FILE = "/etc/kurento/kurento.conf.json";

using namespace ::kurento;

static Service *service;

__pid_t pid;

Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create ();

static gchar *conf_file;
static gchar *tmp_dir;

static GOptionEntry entries[] = {
  {
    "conf-file", 'f', 0, G_OPTION_ARG_FILENAME, &conf_file, "Configuration file",
    NULL
  },
  {NULL}
};

Glib::RefPtr<Glib::IOChannel> channel;

static void
load_config (boost::property_tree::ptree &config, const std::string &file_name)
{
  boost::filesystem::path configFilePath (file_name);
  GST_INFO ("Reading configuration from: %s", file_name.c_str () );

  boost::property_tree::read_json (file_name, config);

  config.add ("configPath", configFilePath.parent_path().string() );
  pid = getpid();

  GST_INFO ("Configuration loaded successfully");

  try {
    service = ServiceFactory::create_service (config);
  } catch (std::exception &e) {
    GST_ERROR ("Error creating service: %s", e.what() );
    exit (1);
  }
}

static void
signal_handler (uint32_t signo)
{
  static unsigned int __terminated = 0;

  switch (signo) {
  case SIGINT:
  case SIGTERM:
    if (__terminated == 0) {
      GST_DEBUG ("Terminating.");
      loop->quit ();
    }

    __terminated = 1;
    break;

  case SIGPIPE:
    GST_DEBUG ("Ignore sigpipe signal");
    break;

  case SIGSEGV:
    GST_DEBUG ("Segmentation fault. Aborting process execution");
    abort ();

  default:
    break;
  }
}

static int
delete_file (const char *fpath, const struct stat *sb, int typeflag,
             struct FTW *ftwbuf)
{
  int rv = g_remove (fpath);

  if (rv) {
    GST_WARNING ("Error deleting file: %s. %s", fpath, strerror (errno) );
  }

  return rv;
}
static void
remove_recursive (const gchar *path)
{
  nftw (path, delete_file, 64, FTW_DEPTH | FTW_PHYS);
}

static void
deleteCertificate ()
{
  // Only parent process can delete certificate
  if (pid != getpid() ) {
    return;
  }

  if (tmp_dir != NULL) {
    remove_recursive (tmp_dir);
    g_free (tmp_dir);
  }
}

int
main (int argc, char **argv)
{
  sigset_t mask;
  std::shared_ptr <SignalHandler> signalHandler;
  GError *error = NULL;
  GOptionContext *context;
  boost::property_tree::ptree config;

  Glib::init();

  gst_init (&argc, &argv);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
                           GST_DEFAULT_NAME);

  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gst_init_get_option_group () );

  if (!g_option_context_parse (context, &argc, &argv, &error) ) {
    g_printerr ("option parsing failed: %s\n", error->message);
    g_option_context_free (context);
    g_error_free (error);
    exit (1);
  }

  g_option_context_free (context);

  /* Install our signal handler */
  sigemptyset (&mask);
  sigaddset (&mask, SIGINT);
  sigaddset (&mask, SIGTERM);
  sigaddset (&mask, SIGSEGV);
  sigaddset (&mask, SIGPIPE);
  signalHandler = std::shared_ptr <SignalHandler> (new SignalHandler (mask,
                  signal_handler) );

  GST_INFO ("Kmsc version: %s", get_version () );

  if (!conf_file) {
    load_config (config, DEFAULT_CONFIG_FILE);
  } else {
    load_config (config, (std::string) conf_file);
  }

  /* Start service */
  service->start ();

  loop->run ();

  signalHandler.reset();

  deleteCertificate ();

  service->stop();

  delete service;

  return 0;
}
