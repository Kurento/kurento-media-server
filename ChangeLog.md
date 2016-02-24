6.4.0
=====

  * Update websocketpp library to version 0.7.0:_
   * This fixes segmentation fault with wss and more than one thread

kms-core (6.4.0)
----------------

  * Prepare implementation to support multistream
  * Fix bad timestamp for opus codec
  * Improve latency stats to add support for multiple streams
  * Fix latency stats calculation
  * Add flow in and flow out signals that indicates if there is media
    going in or out to/from a media element
  * Some fixes in SDP Agent
  * Remb management improvements
  * Add leaky queue in filters to avoid them to buffer media if the proccess
    slower than buffers arrive

kms-elements (6.4.0)
--------------------

  * WebRtcEndpoint: Update libnice library to 0.1.13.1
  * WebRtcEndpoint: Turn is working again now that libnice is updated
  * RecorderEndpoint: Calculate end to end latency stats
  * PlayerEndpoint: Calculate end to end latency
  * WebRtcEndpoint: minor issues fixing
  * RecorderEndpoint: Fix problem when recording to http, now mp4 is buffered
    using and fast start and webm is recorder as live (no seekable without
    post-processing)

kms-filters (6.4.0)
-------------------

  * Update kurento dependencies to 6.4.0

6.3.3
=====

  * Fix installation script

6.3.2
=====

  * Fix problem with writting permission to log folder
  * WebsocketTransport: Fix bug on session injection when there are no parameters

kms-core (6.3.1)
----------------

  * Fix problem with codecs written in lower/upper case
  * Minor code improvements

kms-elements (6.3.1)
--------------------

  * PlayerEndpoint: Fix problem in pause introduced in previous release
  * WebRtcEndpoint: Fix problems with upper/lower case codec names
  * WebRtcEndpoint: Parse candidates present in original offer correctly
  * RecorderEndpoint: Reduce log level for some messages that were not errors

6.3.1
=====

  * Create a kurento user to allow buffering of played medias

6.3.0
=====

  * Remove rabbitmq support
  * Print compilation time and date on log for debugging purposes
  * Fix memory leaks in websockettransport
  * Print stack trace when abort or segfault signals are captured
  * Fix session management in websocket (just one session was used)
  * Add closeSession method to release/dispose all session resources

kms-core
--------

  * SdpEndpoint: Add support for negotiating ipv6 or ipv4
  * Update glib to 2.46
  * SdpEndpoint: Fix bug on missordered medias when they are bunble
  * Add compilation time to module information (makes debugging easier)
  * KurentoException: Add excetions for player
  * agnosticbin: Fix many negotiation problems caused by new empty caps
  * MediaElement/MediaPipeline: Fix segmentation fault when error event is sent
  * agnosticbin: Do not negotiate pad until a reconfigure event is received
    (triying to do so can cause deadlock)
  * sdpAgent: Support mid without a group (Fixes problems with Firefox)
  * Fix problem with REMB notifications when we are sending too much nack events

kms-elements
------------

  * RecorderEndpoint: Fix many problems that appeared with the las gstreamer update
  * RtpEndpoint: Add event to notify when a srtp key is about to expire
  * PlayerEndpoint: Add seek capability
  * WebRtcEndpoint: Fix minor problems with datachannels
  * WebRtcEndpoint: Fix problem with chrome 48 candidates
  * RtpEndpoint: Add support for ipv6, by now only one protocol can be used
  * WebRtcEndpoint: Add tests for ipv6 support
  * WebRtcEndpoint: Do not use TURN configuration until bug in libnice is fixed
    TURN in clients (browsers) can still be used, but kms will not generate
    relay candidates.

kms-filters
-----------

  * Update kurento dependencies to 6.3.0

6.2.0
=====

  * Added ping method, just reply pong
  * Scaffold: Fix installation of configuration files
  
kms-core
--------

  * Update GStreamer version to 1.7
  * RtpEndpoint: Add address in generated SDP. 0.0.0.0 was added so no media
    could return to the server.
  * SdpEndpoint: Add maxAudioRecvBandwidth property
  * BaseRtpEndpoint: Add configuration for port ranges
  * Fixed https://github.com/Kurento/bugtracker/issues/12
  * SdpEndpoint: Raises error when sdp answer cannot be processed
  * MediaPipeline: Add proper error code to error events
  * MediaElement: Fix error notification mechanisms. Errors where not raising in
    most cases
  * UriEndpoint: Add default uri for relative paths. Now uris without schema are
    treated with a default value set in configuration. By default directory
    /var/kurento is used
  * Improvements in format negotiations between elements, this is fixing
    problems in:
    * RecorderEndpoint
    * Composite
  * RecorderEndpoint: Change audio format for WEBM from Vorbis to Opus. This is
    avoiding transcodification and also improving quality.

kms-elements
------------

  * RecorderEndpoint: Fix problems with negative timestamps that produced empty
    videos
  * RtpEndpoint now inherits from BaseRtpEndpoint
  * WebRtcEndpoint uses BaseRtpEndpoint configuration for port ranges
  * RtpEndpoint uses BaseRtpEndpoint configuration for port ranges
  * RecorderEndpoint: Fix negotiation problems with MP4 files. Now format
    changes are not allowed
  * RtpEndpoint: add SDES encryption support
  * RecorderEndpoint: internal redesign simplifying internal pipeline
  * PlayerEndpoint: set correct timestamps when source does not provide them
    properly.
  * WebRtcEndpoint: report possible error on candidate handling
  * Composite: fix bugs simplifying internal design
  
kms-filters
-----------

  * OpenCvFilter: Now exceptions raised in opencv code are sent to the client
      as errors
  * GStreamerFilter: Improve command parser using gst-launch parser
