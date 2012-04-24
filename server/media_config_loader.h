#ifndef __MEDIA_CONFIG_LOADER__
#define __MEDIA_CONFIG_LOADER__

#include <glibmm.h>
#include <sessionSpec_types.h>

#define SERVER_GROUP "Server"

using ::com::kurento::commons::mediaspec::SessionSpec;

void load_spec(Glib::KeyFile &configFile, SessionSpec &session);

void print_spec(SessionSpec &spec);

#endif /* __MEDIA_CONFIG_LOADER__ */
