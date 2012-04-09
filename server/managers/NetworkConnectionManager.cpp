#include "NetworkConnectionManager.h"

using namespace com::kurento::kms;


NetworkConnectionManager::NetworkConnectionManager(
					const MediaSessionImpl& session){
	ms = &session;
}
