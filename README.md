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
  sudo add-apt-repository ppa:kurento/kurento
  sudo apt-add-repository http://ubuntu.kurento.org
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
here:

https://launchpad.net/~kurento/+archive/ubuntu/kurento

To install just execute:

```
  sudo apt-get install software-properties-common # To install add-apt-repository tool
  sudo add-apt-repository ppa:kurento/kurento
  sudo apt-get update
  sudo apt-get install kurento-server
```

You can also find current development version at:

http://ubuntu.kurento.org/pool/main/k/kurento-media-server/

To install packages from unstable repository you need to execute:

```
  sudo apt-get install software-properties-common # To install add-apt-repository tool
  # Ppa is also required for gstreamer packages updates
  sudo add-apt-repository ppa:kurento/kurento
  sudo apt-add-repository http://ubuntu.kurento.org
  wget -O - http://ubuntu.kurento.org/kurento.gpg.key | sudo apt-key add -
  sudo apt-get update
  sudo apt-get install kurento-media-server
```

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

