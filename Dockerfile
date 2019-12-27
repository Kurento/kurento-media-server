# Kurento Media Server
#
# This Docker image is used to run an instance of Kurento Media Server.
#
#
# Build command
# -------------
#
# docker build [Args...] --tag kurento/kurento-media-server:latest .
#
#
# Build arguments
# ---------------
#
# --build-arg UBUNTU_VERSION=<UbuntuVersion>
#
#   <UbuntuVersion> is like "xenial", "bionic", etc.
#
#   Optional. Default: "xenial".
#
# --build-arg KMS_VERSION=<KmsVersion>
#
#   <KmsVersion> is like "6.7.2", "6.9.0", etc.
#   Alternatively, "dev" is used to build a nightly version of KMS.
#
#   Optional. Default: "dev".
#
#
# Run command
# -----------
#
# docker run --name kms -p 8888:8888 kurento/kurento-media-server:latest
#
# Then, you can follow the logs with the `docker logs` command:
#
# docker logs --follow kms >"kms-$(date '+%Y%m%dT%H%M%S').log" 2>&1

ARG UBUNTU_VERSION="xenial"

FROM ubuntu:${UBUNTU_VERSION}

MAINTAINER Patxi Gortázar <patxi.gortazar@gmail.com>
MAINTAINER Fede Diaz <nordri@gmail.com>
MAINTAINER Juan Navarro <juan.navarro@gmx.es>

ARG UBUNTU_VERSION="xenial"
ARG KMS_VERSION="dev"

# Configure environment:
# * DEBIAN_FRONTEND: Disable interactive questions and messages in `apt-get`
# * LANG: Set the default locale for all commands
ENV DEBIAN_FRONTEND="noninteractive" \
    LANG="C.UTF-8"

# Install required tools:
# * gnupg: For `apt-key adv` (since Ubuntu 18.04)
# * curl: For "entrypoint.sh" and "healthchecker.sh"
RUN apt-get update && apt-get install --yes \
        gnupg \
        curl \
 && rm -rf /var/lib/apt/lists/*

# Configure `apt-get`:
# * Disable installation of recommended and suggested packages
# * Add Kurento package repository
RUN echo 'APT::Install-Recommends "false";' >/etc/apt/apt.conf.d/00recommends \
 && echo 'APT::Install-Suggests "false";' >>/etc/apt/apt.conf.d/00recommends \
 && echo "UBUNTU_VERSION=${UBUNTU_VERSION}" \
 && echo "KMS_VERSION=${KMS_VERSION}" \
 && echo "Apt source line: deb [arch=amd64] http://ubuntu.openvidu.io/${KMS_VERSION} ${UBUNTU_VERSION} kms6" \
 && echo "deb [arch=amd64] http://ubuntu.openvidu.io/${KMS_VERSION} ${UBUNTU_VERSION} kms6" >/etc/apt/sources.list.d/kurento.list \
 && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 5AFA7A83

# Install Kurento Media Server
RUN apt-get update && apt-get install --yes \
        kurento-media-server \
 && rm -rf /var/lib/apt/lists/*

# Install additional modules
# These might not be available in Ubuntu 18.04 (Bionic)
RUN apt-get update && apt-get install --yes \
        kms-chroma \
        kms-crowddetector \
        kms-platedetector \
        kms-pointerdetector \
    || true \
 && rm -rf /var/lib/apt/lists/*

# Configure environment for KMS. All of this can be overriden with `docker run`.
# * Use suggested logging levels:
#   https://doc-kurento.readthedocs.io/en/latest/features/logging.html#suggested-levels
# * Disable color in debug logs
# * Provide default empty values for all environment variables supported by
#   the entrypoint script. This acts more as documentation than really a need.
ENV GST_DEBUG="3,Kurento*:4,kms*:4,sdp*:4,webrtc*:4,*rtpendpoint:4,rtp*handler:4,rtpsynchronizer:4,agnosticbin:4" \
    GST_DEBUG_NO_COLOR=1 \
    KMS_MTU="" \
    KMS_EXTERNAL_ADDRESS="" \
    KMS_NETWORK_INTERFACES="" \
    KMS_STUN_IP="" \
    KMS_STUN_PORT="" \
    KMS_TURN_URL=""

# Expose default Kurento RPC control port
EXPOSE 8888

COPY ./entrypoint.sh /entrypoint.sh
COPY ./healthchecker.sh /healthchecker.sh

HEALTHCHECK --start-period=15s --interval=30s --timeout=3s --retries=1 CMD /healthchecker.sh

ENTRYPOINT ["/entrypoint.sh"]
