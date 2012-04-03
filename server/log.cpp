#include "log.h"
#include <glibmm.h>

using com::kurento::log::Log;

Log::Log(std::string domain) {
	this->domain = domain;
}

Log::~Log() {
}

static char *
append_log_formats(int line, std::string function, std::string file,
					std::string format, va_list args) {
	// TODO: We need a system to select the log format dynamically
	char *message;
	char *formmated_message;

	message = g_strdup_vprintf(format.c_str(), args);

	formmated_message = g_strdup_printf("%s:%d %s", file.c_str(),
							line, message);
	g_free(message);

	return formmated_message;
}

void
Log::debug(std::string format, ... ) {
	va_list args;

	va_start(args, format);
	g_log(domain.c_str(), G_LOG_LEVEL_DEBUG, format.c_str(), args);
	va_end(args);
}

void
Log::debug(int line, std::string function, std::string file,
						std::string format, ... ) {
	char *message;
	va_list args;

	va_start(args, format);
	message = append_log_formats(line, function, file, format, args);
	g_log(domain.c_str(), G_LOG_LEVEL_DEBUG, message);
	g_free(message);
	va_end(args);
}

void
Log::info(std::string format, ... ) {
	va_list args;

	va_start(args, format);
	g_log(domain.c_str(), G_LOG_LEVEL_INFO, format.c_str(), args);
	va_end(args);
}

void
Log::info(int line, std::string function, std::string file,
						std::string format, ... ) {
	char *message;
	va_list args;

	va_start(args, format);
	message = append_log_formats(line, function, file, format, args);
	g_log(domain.c_str(), G_LOG_LEVEL_INFO, message);
	g_free(message);
	va_end(args);
}
