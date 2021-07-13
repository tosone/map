FROM debian:buster-slim

ENV MUSL_VERSION 1.2.2

RUN set -eux; \
    # sed -i "s/deb.debian.org/mirrors.aliyun.com/g" /etc/apt/sources.list && \
    # sed -i "s/security.debian.org/mirrors.aliyun.com/g" /etc/apt/sources.list && \
    apt-get update; \
    apt-get install -y --no-install-recommends \
    ca-certificates \
    gnupg dirmngr \
    wget \
    \
    gcc \
    libc6-dev \
    make \
    python2 \
    python3 \
    \
    libc6-dev-arm64-cross \
    libc6-dev-armel-cross \
    libc6-dev-armhf-cross \
    libc6-dev-i386-cross \
    libc6-dev-ppc64el-cross \
    libc6-dev-s390x-cross \
    \
    gcc-aarch64-linux-gnu \
    gcc-arm-linux-gnueabi \
    gcc-arm-linux-gnueabihf \
    gcc-i686-linux-gnu \
    gcc-powerpc64le-linux-gnu \
    gcc-s390x-linux-gnu \
    \
    file \
    ; \
    rm -rf /var/lib/apt/lists/* && \
    \
    wget -O musl.tgz.asc "https://www.musl-libc.org/releases/musl-$MUSL_VERSION.tar.gz.asc"; \
    wget -O musl.tgz "https://www.musl-libc.org/releases/musl-$MUSL_VERSION.tar.gz"; \
    \
    export GNUPGHOME="$(mktemp -d)"; \
    gpg --batch --keyserver keyserver.ubuntu.com --recv-keys '836489290BB6B70F99FFDA0556BCDB593020450F'; \
    gpg --batch --verify musl.tgz.asc musl.tgz; \
    gpgconf --kill all; \
    rm -rf "$GNUPGHOME" musl.tgz.asc; \
    \
    mkdir /usr/local/src/musl; \
    tar --extract --file musl.tgz --directory /usr/local/src/musl --strip-components 1; \
    rm musl.tgz

WORKDIR /map

COPY . .

RUN if [ -d musl ]; then rm -rf musl; fi && \
    mkdir -p musl/arm32v5 && cd musl/arm32v5 && \
    CROSS_COMPILE=arm-linux-gnueabi- TARGET_ARCH=arm32v5 \
    /usr/local/src/musl/configure --disable-shared --prefix=/map/musl/arm32v5/prefix > /dev/null && \
    make install -j8 > /dev/null && cd ../.. && make clean && \
    ARCH=arm32v5 CROSS_COMPILE=arm-linux-gnueabi- \
    CC=/map/musl/arm32v5/prefix/bin/musl-gcc make deps all && \
    mv map-arm32v5 /usr/local/bin && \
    \
    mkdir -p musl/amd64 && cd musl/amd64 && \
    CROSS_COMPILE=x86_64-linux-gnu- TARGET_ARCH=amd64 \
    /usr/local/src/musl/configure --disable-shared --prefix=/map/musl/amd64/prefix > /dev/null && \
    make install -j8 > /dev/null && cd ../.. && make clean && \
    ARCH=amd64 CROSS_COMPILE=x86_64-linux-gnu- \
    CC=/map/musl/amd64/prefix/bin/musl-gcc make deps all && \
    mv map-amd64 /usr/local/bin && \
    \
    mkdir -p musl/arm32v7 && cd musl/arm32v7 && \
    CROSS_COMPILE=aarch64-linux-gnu- TARGET_ARCH=arm32v7 \
    /usr/local/src/musl/configure --disable-shared --prefix=/map/musl/arm32v7/prefix > /dev/null && \
    make install -j8 > /dev/null && cd ../.. && make clean && \
    ARCH=arm32v7 CROSS_COMPILE=aarch64-linux-gnu- \
    CC=/map/musl/arm32v7/prefix/bin/musl-gcc make deps all && \
    mv map-arm32v7 /usr/local/bin && \
    \
    mkdir -p musl/arm64v8 && cd musl/arm64v8 && \
    CROSS_COMPILE=arm-linux-gnueabihf- TARGET_ARCH=arm64v8 \
    /usr/local/src/musl/configure --disable-shared --prefix=/map/musl/arm64v8/prefix > /dev/null && \
    make install -j8 > /dev/null && cd ../.. && make clean && \
    ARCH=arm64v8 CROSS_COMPILE=arm-linux-gnueabihf- \
    CC=/map/musl/arm64v8/prefix/bin/musl-gcc make deps all && \
    mv map-arm64v8 /usr/local/bin && \
    \
    mkdir -p musl/i386 && cd musl/i386 && \
    CROSS_COMPILE=i686-linux-gnu- TARGET_ARCH=i386 \
    /usr/local/src/musl/configure --disable-shared --prefix=/map/musl/i386/prefix > /dev/null && \
    make install -j8 > /dev/null && cd ../.. && make clean && \
    ARCH=i386 CROSS_COMPILE=i686-linux-gnu- \
    CC=/map/musl/i386/prefix/bin/musl-gcc make deps all && \
    mv map-i386 /usr/local/bin && \
    \
    mkdir -p musl/ppc64le && cd musl/ppc64le && \
    CROSS_COMPILE=powerpc64le-linux-gnu- TARGET_ARCH=ppc64le CFLAGS=-mlong-double-64 \
    /usr/local/src/musl/configure --disable-shared --prefix=/map/musl/ppc64le/prefix > /dev/null && \
    make install -j8 > /dev/null && cd ../.. && make clean && \
    ARCH=ppc64le CROSS_COMPILE=powerpc64le-linux-gnu- \
    CC=/map/musl/ppc64le/prefix/bin/musl-gcc make deps all && \
    mv map-ppc64le /usr/local/bin && \
    \
    mkdir -p musl/s390x && cd musl/s390x && \
    CROSS_COMPILE=s390x-linux-gnu- TARGET_ARCH=s390x \
    /usr/local/src/musl/configure --disable-shared --prefix=/map/musl/s390x/prefix > /dev/null && \
    make install -j8 > /dev/null && cd ../.. && make clean && \
    ARCH=s390x CROSS_COMPILE=s390x-linux-gnu- \
    CC=/map/musl/s390x/prefix/bin/musl-gcc make deps all && \
    mv map-s390x /usr/local/bin
