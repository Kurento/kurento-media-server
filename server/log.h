/*
kmsc - Kurento Media Server C/C++ implementation
Copyright (C) 2012 Tikal Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KMSC_LOG
#define KMSC_LOG

#include <glibmm.h>

namespace com { namespace kurento { namespace log {

#define aux_debug(log, ...) log.debug(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define aux_info(log, ...) log.info(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
// #define aux_error(log, ...) log.error(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
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