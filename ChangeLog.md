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
