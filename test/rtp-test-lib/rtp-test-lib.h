#ifndef __RTP_TEST_LIB__
#define __RTP_TEST_LIB__

#include <kms-core.h>

KmsEndpoint* create_endpoint();
void check_endpoint(KmsEndpoint *ep);

KmsSdpSession* create_second_session();

#endif /* __RTP_TEST_LIB__ */
