# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [6.6.2] - 2017-07-24

### Changed
- Old ChangeLog.md moved to the new format in this CHANGELOG.md file.
- CMake: Full review of all CMakeLists.txt files to tidy up and homogenize code style and compiler flags.
- CMake: Position Independent Code flags ("-fPIC") were scattered around projects, and are now removed. Instead, the more CMake-idiomatic variable "CMAKE_POSITION_INDEPENDENT_CODE" is used.
- CMake: All projects now compile with "[-std=c11|-std=c++11] -Wall -Werror -pthread".
- CMake: Debug builds now compile with "-g -O0" (while default CMake used "-O1" for Debug builds).
- CMake: include() and import() commands were moved to the code areas where they are actually required.

### Fixed
- Fix missing header in "server/loadConfig.cpp".

## [6.6.1] - 2016-09-30

### Changed
- Fixes on tests.
- Improve compilation process.

## [6.6.0] - 2016-09-09

### Fixed
- Minor compilation warnings.
- Fix resource limits checking; if a limit is not configured then the check wasn't being done.

## [6.5.0] - 2016-05-27

### Changed
- Changed license to Apache 2.0.
- Updated documentation.
- Improve performance of RPC proccessing.
- Allow qualified names for types.

### Fixed
- Bug on client reconnection (they thought that the reconnection succeed even if it was a diferent server).

## [6.4.0] - 2016-02-24

### Fixed
- Update websocketpp library to version 0.7.0. This fixes segmentation fault with wss and more than one thread.

## [6.3.3] - 2016-02-01

### Fixed
- Installation script.

## [6.3.2] - 2016-01-29

### Fixed
- Problem with write permissions to log folder.
- WebsocketTransport: Fix bug on session injection when there are no parameters.

## [6.3.1] - 2016-01-20

### Changed
- Create a kurento user to allow buffering of played medias.

## [6.3.0] - 2019-01-19

### Added
- Print compilation time and date on log for debugging purposes.
- Print stack trace when abort or segfault signals are captured.
- Add "closeSession" method to release/dispose all session resources.

### Removed
- Support for RabbitMQ.

### Fixed
- Fix memory leaks in websockettransport.
- Fix session management in websocket (just one session was used).

## 6.2.0 - 2015-11-25

### Added
- New Ping/Pong based protocol for keeping connections alive.

### Fixed
- Scaffold: Fix installation of configuration files.

[6.6.2]: https://github.com/Kurento/kurento-media-server/compare/6.6.1...6.6.2
[6.6.1]: https://github.com/Kurento/kurento-media-server/compare/6.6.0...6.6.1
[6.6.0]: https://github.com/Kurento/kurento-media-server/compare/6.5.0...6.6.0
[6.5.0]: https://github.com/Kurento/kurento-media-server/compare/6.4.0...6.5.0
[6.4.0]: https://github.com/Kurento/kurento-media-server/compare/6.3.3...6.4.0
[6.3.3]: https://github.com/Kurento/kurento-media-server/compare/6.3.2...6.3.3
[6.3.2]: https://github.com/Kurento/kurento-media-server/compare/6.3.1...6.3.2
[6.3.1]: https://github.com/Kurento/kurento-media-server/compare/6.3.0...6.3.1
[6.3.0]: https://github.com/Kurento/kurento-media-server/compare/6.2.0...6.3.0
