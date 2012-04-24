#include "log.h"
#include <glibmm.h>

using com::kurento::log::Log;

static void
log_info(const gchar *log_domain,
	    GLogLevelFlags log_level,
	    const gchar *message,
	    gpointer user_data) {
	g_print("INFO: %s: %s\n", log_domain, message);
}

static void
log_debug(const gchar *log_domain,
	    GLogLevelFlags log_level,
	    const gchar *message,
	    gpointer user_data) {
	g_print("DEBUG: %s: %s\n", log_domain, message);
}

static void
log_warning(const gchar *log_domain,
	    GLogLevelFlags log_level,
	    const gchar *message,
	    gpointer user_data) {
	g_print("**WARNING**: %s: %s\n\n", log_domain, message);
}

Log::Log(std::string domain) {
	this->domain = domain;

	g_log_set_handler (domain.c_str(), (GLogLevelFlags) (G_LOG_LEVEL_INFO),
				log_info, NULL);
	g_log_set_handler (domain.c_str(), (GLogLevelFlags) (G_LOG_LEVEL_DEBUG),
				log_debug, NULL);
	g_log_set_handler (domain.c_str(), (GLogLevelFlags) (G_LOG_LEVEL_WARNING),
			   log_warning, NULL);
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

// 	formmated_message = g_strdup_printf("%s:%d %s", file.c_str(),
// 							line, message);
	formmated_message = g_strdup_printf("#%d %s", line, message);
	g_free(message);

	return formmated_message;
}

void
Log::debug(std::string format, ... ) {
	va_list args;

	va_start(args, format);
	g_logv(domain.c_str(), G_LOG_LEVEL_DEBUG, format.c_str(), args);
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
	g_logv(domain.c_str(), G_LOG_LEVEL_INFO, format.c_str(), args);
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

void
Log::error(std::string format, ... ) {
	va_list args;

	va_start(args, format);
	g_logv(domain.c_str(), G_LOG_LEVEL_ERROR, format.c_str(), args);
	va_end(args);
}

void
Log::error(int line, std::string function, std::string file,
						std::string format, ... ) {
	char *message;
	va_list args;

	va_start(args, format);
	message = append_log_formats(line, function, file, format, args);
	g_log(domain.c_str(), G_LOG_LEVEL_ERROR, message);
	g_free(message);
	va_end(args);
}

void
Log::warn(std::string format, ... ) {
	va_list args;

	va_start(args, format);
	g_logv(domain.c_str(), G_LOG_LEVEL_WARNING, format.c_str(), args);
	va_end(args);
}

void
Log::warn(int line, std::string function, std::string file,
						std::string format, ... ) {
	char *message;
	va_list args;

	va_start(args, format);
	message = append_log_formats(line, function, file, format, args);
	g_log(domain.c_str(), G_LOG_LEVEL_WARNING, message);
	g_free(message);
	va_end(args);
}
