#include <kms-core.h>
#include <thrift/transport/thrift_memory_buffer.h>
#include <thrift/protocol/thrift_binary_protocol.h>

void
kms_session_spec_intersect(KmsSessionSpec *answerer, KmsSessionSpec *offerer,
						KmsSessionSpec **neg_answ,
						KmsSessionSpec **neg_off) {

	*neg_off = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);
	*neg_answ = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);
}

KmsSessionSpec*
kms_session_spec_copy(KmsSessionSpec *self) {
	ThriftMemoryBuffer *transport;
	ThriftProtocol *protocol;
	KmsSessionSpec *spec;
	GError *err = NULL;

	transport = g_object_new(THRIFT_TYPE_MEMORY_BUFFER,
							"buf_size", 2048, NULL);
	protocol = g_object_new(THRIFT_TYPE_BINARY_PROTOCOL,
						"transport", transport, NULL);
	if (thrift_struct_write(THRIFT_STRUCT(self), protocol, &err) < 0) {
		if (err != NULL) {
			g_print("Error on write %s\n", err->message);
			g_error_free(err);
		}
		goto end;
	}

	spec = g_object_new(KMS_TYPE_SESSION_SPEC, NULL);
	if (thrift_struct_read(THRIFT_STRUCT(spec), protocol, &err) < 0) {
		if (err != NULL) {
			g_print("Error on write %s\n", err->message);
			g_error_free(err);
		}
		g_object_unref(spec);
		spec = NULL;
		goto end;
	};
end:
	g_object_unref(transport);
	g_object_unref(protocol);
	return spec;
}
