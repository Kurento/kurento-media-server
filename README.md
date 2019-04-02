<a href="https://www.kurento.org/">
    <img src="https://secure.gravatar.com/avatar/21a2a12c56b2a91c8918d5779f1778bf?s=120">
</a>

Kurento Media Server
====================

[![FIWARE Chapter](https://nexus.lab.fiware.org/repository/raw/public/badges/chapters/media-streams.svg)](https://www.fiware.org/developers/catalogue/)
[![License badge](https://img.shields.io/badge/license-Apache2-orange.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Build Status](https://ci.openvidu.io/jenkins/buildStatus/icon?job=Development/kurento_media_server_merged_xenial)]()
[![Docker badge](https://img.shields.io/docker/pulls/fiware/orion.svg)](https://hub.docker.com/r/kurento/kurento-media-server)
[![Support badge]( https://img.shields.io/badge/tag-Kurento-orange.svg?logo=stackoverflow)](http://stackoverflow.com/questions/tagged/kurento)
<br/>
[![Documentation badge](https://readthedocs.org/projects/fiware-orion/badge/?version=latest)](https://doc-kurento.readthedocs.io)
[![FIWARE member status](https://nexus.lab.fiware.org/static/badges/statuses/kurento.svg)](https://www.fiware.org/developers/catalogue/)

Kurento Media Server is Kurento's core element. It is responsible for media transmission, processing, loading and recording. It is implemented in low level technologies based on GStreamer to optimize the resource consumption. It provides the following features:

* Networked streaming protocols, including HTTP, RTP and WebRTC.
* Group communications (MCUs and SFUs functionality) supporting both media mixing and media routing/dispatching.
* Generic support for computational vision and augmented reality filters.
* Media storage supporting writing operations for WebM and MP4 and playing in all formats supported by GStreamer.
* Automatic media transcoding between any of the codecs supported by GStreamer including VP8, H.264, H.263, AMR, OPUS, Speex, G.711, etc.


This project is part of [FIWARE](https://www.fiware.org/). For more information check the FIWARE Catalogue entry for
[Context Processing, Analysis and Visualization](https://github.com/Fiware/catalogue/tree/master/processing).

| :books:[Documentation](https://www.kurento.org/documentation) | :page_facing_up: [Site](https://www.kurento.org/) | :mortar_board: [Academy](https://fiware-academy.readthedocs.io/en/latest/processing/kurento) | :whale: [Docker Hub](https://hub.docker.com/r/kurento/kurento-media-server) |
| ---|---|---|---|



About Kurento
=============

Kurento is an open source software project providing a platform suitable for creating modular applications with advanced real-time communication capabilities. For knowing more about Kurento, please visit the Kurento project website: https://www.kurento.org/.

Kurento is part of [FIWARE]. For further information on the relationship of FIWARE and Kurento check the [Kurento FIWARE Catalog Entry].

Kurento has been rated within [FIWARE] as follows:

-   **Version Tested:**
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Version&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.version&colorB=blue)
-   **Documentation:**
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Completeness&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.docCompleteness&colorB=blue)
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Usability&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.docSoundness&colorB=blue)
-   **Responsiveness:**
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Time%20to%20Respond&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.timeToCharge&colorB=blue)
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Time%20to%20Fix&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.timeToFix&colorB=blue)
-   **FIWARE Testing:**
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Tests%20Passed&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.failureRate&colorB=blue)
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Scalability&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.scalability&colorB=blue)
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Performance&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.performance&colorB=blue)
    ![ ](https://img.shields.io/badge/dynamic/json.svg?label=Stability&url=https://fiware.github.io/catalogue/json/kurento.json&query=$.stability&colorB=blue)


Kurento is also part of the [NUBOMEDIA] research initiative.

[FIWARE]: http://www.fiware.org
[Kurento FIWARE Catalog Entry]: http://catalogue.fiware.org/enablers/stream-oriented-kurento
[NUBOMEDIA]: http://www.nubomedia.eu



Documentation
-------------

The Kurento project provides detailed [documentation] including tutorials, installation and development guides. The [Open API specification], also known as *Kurento Protocol*, is available on [apiary.io].

[documentation]: https://www.kurento.org/documentation
[Open API specification]: https://doc-kurento.readthedocs.io/en/stable/features/kurento_api.html
[apiary.io]: http://docs.streamoriented.apiary.io/



Useful Links
------------

Usage:

* [Installation Guide](https://doc-kurento.readthedocs.io/en/stable/user/installation.html)
* [Docker Deployment Guide](https://hub.docker.com/r/kurento/kurento-media-server)
* [Contribution Guide](https://doc-kurento.readthedocs.io/en/stable/project/contribute.html)
* [Developer Guide](https://doc-kurento.readthedocs.io/en/stable/dev/dev_guide.html)

Issues:

* [Bug Tracker](https://github.com/Kurento/bugtracker/issues)
* [Support](https://doc-kurento.readthedocs.io/en/stable/user/support.html)

News:

* [Kurento Blog](https://www.kurento.org/blog)
* [Google Groups](https://groups.google.com/forum/#!forum/kurento)
* [Kurento RoadMap](https://github.com/Kurento/kurento-media-server/blob/master/ROADMAP.md)

Training:

* [Kurento tutorials](https://doc-kurento.readthedocs.io/en/stable/user/tutorials.html)



Source
------

All source code belonging to the Kurento project can be found in the [Kurento GitHub organization page].

[Kurento GitHub organization page]: https://github.com/Kurento



Testing
-------

Kurento has a full set of different tests mainly focused in the integrated and system tests, more specifically e2e tests that anyone can run to assess different parts of Kurento, namely functional, stability, tutorials, and API.

In order to assess properly Kurento from a final user perspective, a rich suite of E2E tests has been designed and implemented. To that aim, the Kurento Testing Framework (KTF) has been created. KTF is a part of the Kurento project aimed to carry out end-to-end (E2E) tests for Kurento. KTF has been implemented on the top of two well-known open-source testing frameworks: JUnit and Selenium.

If you want to know more about the Kurento Testing Framework and how to run all the available tests for Kurento you will find more information in [Kurento developers documentation > Testing]

[Kurento developers documentation > Testing]: https://doc-kurento.readthedocs.io/en/stable/dev/testing.html



Licensing and distribution
--------------------------

Copyright 2019 Kurento

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
