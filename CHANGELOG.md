# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [6.8.1] - 2018-10-23

### Fixed
- Service init files will now append to the error log file, instead of truncating it on each restart.

## [6.8.0] - 2018-09-26

### Added
- GLib logging messages are now integrated with Kurento logging. This means that it's possible to save debug messages from libnice, together with the usual logs from Kurento / GStreamer.

### Changed
- Output logs now use standard format ISO 8601.
- `disableRequestCache` is now exposed in settings file (*kurento.conf.json*). This can be used to disable the RPC Request Cache.
- Clearer log messages about what is going on when the maximum resource usage threshold is reached.
- File `/etc/default/kurento-media-server` now contains more useful examples and explanations for each option.

## [6.7.2] - 2018-05-11

### Fixed
- All: Apply multiple fixes suggested by *clang-tidy*.
- Re-add redirection of 'stderr': log DeathHandler messages.
- [#245](https://github.com/Kurento/bugtracker/issues/245) (Possible SYN flooding): WebSocketTransport: Change default listen backlog to `socket_base::max_connections`.
- [#242](https://github.com/Kurento/bugtracker/issues/242) (libSSL crashes on mirrored packets): Debian: Remove dependency on our unmaintained fork of libSSL - **Work In Progress**.
- Debian: Remove unneeded build dependency: binutils.
- Debian: Use better defaults for logging levels.

## [6.7.1] - 2018-03-21

### Changed
- Raise version to 6.7.1.

## [6.7.0] - 2018-01-24

### Changed
- CMake: Compile and link as Position Independent Executable ('-fPIE -pie').
- Add more verbose logging in some areas that required it.
- Debian: Align all version numbers of KMS-related modules.
- Debian: Remove version numbers from package names.
- Debian: Configure builds to use parallel compilation jobs.

### Fixed

- Remove usage of 'sudo' from init script.

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

[6.8.1]: https://github.com/Kurento/kurento-media-server/compare/6.8.0...6.8.1
[6.8.0]: https://github.com/Kurento/kurento-media-server/compare/6.7.2...6.8.0
[6.7.2]: https://github.com/Kurento/kurento-media-server/compare/6.7.1...6.7.2
[6.7.1]: https://github.com/Kurento/kurento-media-server/compare/6.7.0...6.7.1
[6.7.0]: https://github.com/Kurento/kurento-media-server/compare/6.6.2...6.7.0
[6.6.2]: https://github.com/Kurento/kurento-media-server/compare/6.6.1...6.6.2
[6.6.1]: https://github.com/Kurento/kurento-media-server/compare/6.6.0...6.6.1
[6.6.0]: https://github.com/Kurento/kurento-media-server/compare/6.5.0...6.6.0
[6.5.0]: https://github.com/Kurento/kurento-media-server/compare/6.4.0...6.5.0
[6.4.0]: https://github.com/Kurento/kurento-media-server/compare/6.3.3...6.4.0
[6.3.3]: https://github.com/Kurento/kurento-media-server/compare/6.3.2...6.3.3
[6.3.2]: https://github.com/Kurento/kurento-media-server/compare/6.3.1...6.3.2
[6.3.1]: https://github.com/Kurento/kurento-media-server/compare/6.3.0...6.3.1
[6.3.0]: https://github.com/Kurento/kurento-media-server/compare/6.2.0...6.3.0
