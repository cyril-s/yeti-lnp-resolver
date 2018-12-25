FROM debian:stretch

ENV DEBIAN_FRONTEND=noninteractive LANG=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends gnupg wget

RUN echo "deb http://pkg.yeti-switch.org/debian/stretch unstable main" >> /etc/apt/sources.list && \
    wget -O - http://pkg.yeti-switch.org/key.gpg | apt-key add -

RUN apt-get update && apt-get install -y --no-install-recommends \
       build-essential \
       devscripts \
       fakeroot \
       lintian \
       debhelper \
       cmake \
       git \
       libcurl4-gnutls-dev \
       pkg-config \
       libnanomsg-dev \
       libconfuse-dev \
       libyeticc-dev \
       libpqxx-dev \
       vim \
       python3-pip \
       python3-setuptools
#    apt-get clean && rm -rf /var/lib/apt/lists/*

RUN pip3 install aptly-ctl

ADD . /build/
WORKDIR /build/

RUN find -name '*.swp' -delete
