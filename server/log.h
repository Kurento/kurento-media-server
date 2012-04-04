#ifndef KMSC_LOG
#define KMSC_LOG

#include <glibmm.h>

namespace com { namespace kurento { namespace log {

#define aux_debug(log, ...) log.debug(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define aux_info(log, ...) log.info(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define aux_error(log, ...) log.error(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define aux_warn(log, ...) log.warn(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)

class Log {
public:
	Log(std::string domain);
	~Log();

	void debug(std::string format, ...);
	void debug(int line, std::string function, std::string file, std::string format, ...);

	void info(std::string format, ...);
	void info(int line, std::string function, std::string file, std::string format, ...);

	void error(std::string format, ...);
	void error(int line, std::string function, std::string file, std::string format, ...);

	void warn(std::string format, ...);
	void warn(int line, std::string function, std::string file, std::string format, ...);

private:

	std::string domain;
};

}}} // com::kurento::log

#endif /* KMSC_LOG */