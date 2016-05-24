[![License badge](https://img.shields.io/badge/license-Apache2-orange.svg)](http://www.apache.org/licenses/LICENSE-2.0)
[![Documentation badge](https://readthedocs.org/projects/fiware-orion/badge/?version=latest)](http://doc-kurento.readthedocs.org/en/latest/)
[![Docker badge](https://img.shields.io/docker/pulls/fiware/orion.svg)](https://hub.docker.com/r/fiware/stream-oriented-kurento/)
[![Support badge]( https://img.shields.io/badge/support-sof-yellowgreen.svg)](http://stackoverflow.com/questions/tagged/kurento)

[![][KurentoImage]][Kurento]

Copyright © 2013-2016 [Kurento]. Licensed under [Apache 2.0 License].

kurento-media-server
===================

Kurento Media Server is the Kurento's core element. It is responsible for media
transmission, processing, loading and recording. It is implemented in low level
technologies based on GStreamer to optimize the resource consumption. It
provides the following features:

* Networked streaming protocols, including HTTP, RTP and WebRTC.

* Group communications (MCUs and SFUs functionality) supporting both media
  mixing and media routing/dispatching.

* Generic support for computational vision and augmented reality filters.

* Media storage supporting writing operations for WebM and MP4 and playing in
  all formats supported by GStreamer.

* Automatic media transcodification between any of the codecs supported by
  GStreamer including VP8, H.264, H.263, AMR, OPUS, Speex, G.711, etc.

Compilation instructions
------------------------

First you need to install dependencies, there are various ways, here we describe
one that is pretty simple and automated:

```
  echo "deb http://ubuntu.kurento.org trusty-dev kms6" | sudo tee /etc/apt/sources.list.d/kurento-dev.list
  wget -O - http://ubuntu.kurento.org/kurento.gpg.key | sudo apt-key add -
  sudo apt-get update
  sudo apt-get install $(cat debian/control | sed -e "s/$/\!\!/g" | tr -d '\n' | sed "s/\!\! / /g" | sed "s/\!\!/\n/g" | grep "Build-Depends" | sed "s/Build-Depends: //g" | sed "s/([^)]*)//g" | sed "s/, */ /g")
```

Then you can compile the server, just execute:

```
  mkdir -p build
  cd build
  cmake ..
  make -j4
```

If you want to create a debian package you can execute:

```
  sudo apt-get install devscripts
  git submodule update --init --recursive
  debuild -us -uc
```

Debian packages will be created on parent directory.

Binary distribution
-------------------

This software is distributed as debian packages, you can find stable releases
here using our debian repository.

To install just execute:

```
  echo "deb http://ubuntu.kurento.org trusty kms6" | sudo tee /etc/apt/sources.list.d/kurento.list
  wget -O - http://ubuntu.kurento.org/kurento.gpg.key | sudo apt-key add -
  sudo apt-get update
  sudo apt-get install kurento-server
```

You can also find current development version at:

http://ubuntu.kurento.org/pool/main/k/kurento-media-server/

To install packages from unstable repository you need to execute:

```
  echo "deb http://ubuntu.kurento.org trusty-dev kms6" | sudo tee /etc/apt/sources.list.d/kurento-dev.list
  wget -O - http://ubuntu.kurento.org/kurento.gpg.key | sudo apt-key add -
  sudo apt-get update
  sudo apt-get install kurento-media-server-6.0
```

Kurento
=======

What is Kurento
---------------

Kurento is an open source software project providing a platform suitable 
for creating modular applications with advanced real-time communication
capabilities. For knowing more about Kurento, please visit the Kurento
project website: http://www.kurento.org.

Kurento is part of [FIWARE]. For further information on the relationship of 
FIWARE and Kurento check the [Kurento FIWARE Catalog Entry]

Kurento is part of the [NUBOMEDIA] research initiative.

Documentation
-------------

The Kurento project provides detailed [documentation] including tutorials,
installation and development guides. A simplified version of the documentation
can be found on [readthedocs.org]. The [Open API specification] a.k.a. Kurento
Protocol is also available on [apiary.io].

Source
------

Code for other Kurento projects can be found in the [GitHub Kurento Group].

News and Website
----------------

Check the [Kurento blog]
Follow us on Twitter @[kurentoms].

Issue tracker
-------------

Issues and bug reports should be posted to the [GitHub Kurento bugtracker]

Licensing and distribution
--------------------------

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Contribution policy
-------------------

You can contribute to the Kurento community through bug-reports, bug-fixes, new
code or new documentation. For contributing to the Kurento community, drop a
post to the [Kurento Public Mailing List] providing full information about your
contribution and its value. In your contributions, you must comply with the
following guidelines

* You must specify the specific contents of your contribution either through a
  detailed bug description, through a pull-request or through a patch.
* You must specify the licensing restrictions of the code you contribute.
* For newly created code to be incorporated in the Kurento code-base, you must
  accept Kurento to own the code copyright, so that its open source nature is
  guaranteed.
* You must justify appropriately the need and value of your contribution. The
  Kurento project has no obligations in relation to accepting contributions
  from third parties.
* The Kurento project leaders have the right of asking for further
  explanations, tests or validations of any code contributed to the community
  before it being incorporated into the Kurento code-base. You must be ready to
  addressing all these kind of concerns before having your code approved.

Support
-------

The Kurento project provides community support through the  [Kurento Public
Mailing List] and through [StackOverflow] using the tags *kurento* and
*fiware-kurento*.

Before asking for support, please read first the [Kurento Netiquette Guidelines]

[documentation]: http://www.kurento.org/documentation
[FIWARE]: http://www.fiware.org
[GitHub Kurento bugtracker]: https://github.com/Kurento/bugtracker/issues
[GitHub Kurento Group]: https://github.com/kurento
[kurentoms]: http://twitter.com/kurentoms
[Kurento]: http://kurento.org
[Kurento Blog]: http://www.kurento.org/blog
[Kurento FIWARE Catalog Entry]: http://catalogue.fiware.org/enablers/stream-oriented-kurento
[Kurento Netiquette Guidelines]: http://www.kurento.org/blog/kurento-netiquette-guidelines
[Kurento Public Mailing list]: https://groups.google.com/forum/#!forum/kurento
[KurentoImage]: https://secure.gravatar.com/avatar/21a2a12c56b2a91c8918d5779f1778bf?s=120
[Apache 2.0 License]: http://www.apache.org/licenses/LICENSE-2.0
[NUBOMEDIA]: http://www.nubomedia.eu
[StackOverflow]: http://stackoverflow.com/search?q=kurento
[Read-the-docs]: http://read-the-docs.readthedocs.org/
[readthedocs.org]: http://kurento.readthedocs.org/
[Open API specification]: http://kurento.github.io/doc-kurento/
[apiary.io]: http://docs.streamoriented.apiary.io/
