[![][KurentoImage]][website]

Copyright © 2013 Kurento. Licensed under [LGPL License].

kurento-media-server
==========
Kurento Media Server

Kurento Media Server: Kurento Media Server processes audio and video streams,
doing composable pipeline-based processing of media.

Compilation instructions
------

First you need to install dependencies, there are various ways, here we describe
one that is pretty simple and automated:

```
  sudo apt-get install devscripts
  sudo apt-get update
  pkgs=`dpkg-checkbuilddeps 2>&1` || echo "Installing dependencies"
  pkgs=$(echo $pkgs | sed "s/dpkg-checkbuilddeps: Unmet build dependencies: //g" | sed "s/([^)]*)//g"|sed "s/,/ /g" )
  sudo apt-get install $pkgs
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

Source
------
The source code of this project can be cloned from the [GitHub Repository].
Code for other Kurento projects can be found in the [GitHub Kurento Group].

News and Website
----------------
Information about Kurento can be found on our [website].
Follow us on Twitter @[kurentoms].

[KurentoImage]: https://avatars1.githubusercontent.com/u/5392839?s=120
[LGPL License]: http://www.gnu.org/licenses/lgpl-2.1.html
[GitHub Repository]: https://github.com/kurento/kurento-media-server
[GitHub Kurento Group]: https://github.com/kurento
[website]: http://kurento.org
[kurentoms]: http://twitter.com/kurentoms

