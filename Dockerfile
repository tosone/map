# Cross compile static musl binaries for 2 architectures using zig's bundled musl
#
# architecture mapping (zig target names):
#   amd64   -> x86_64-linux-musl
#   arm64v8 -> aarch64-linux-musl
#
# Note: RUN lines are handed to /bin/sh as is (no $$ escaping), so use
#       $VAR / $(...) directly and never write $$.
FROM debian:trixie-slim

ENV ZIG_VERSION=0.16.0

# Tool dependencies:
#   binutils-multiarch - the default binutils strip only understands the host
#                        architecture, the multiarch build strips any ELF
RUN set -eux; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        wget \
        xz-utils \
        make \
        binutils \
        binutils-multiarch \
        file \
    ; \
    rm -rf /var/lib/apt/lists/*; \
    \
    case "$(uname -m)" in \
        x86_64|amd64)  zig_host=x86_64 ;; \
        aarch64|arm64) zig_host=aarch64 ;; \
        *) echo "unsupported host arch: $(uname -m)" >&2; exit 1 ;; \
    esac; \
    wget -O /tmp/zig.tar.xz \
        "https://ziglang.org/download/${ZIG_VERSION}/zig-${zig_host}-linux-${ZIG_VERSION}.tar.xz"; \
    tar -C /opt -xf /tmp/zig.tar.xz; \
    rm /tmp/zig.tar.xz; \
    mv "/opt/zig-${zig_host}-linux-${ZIG_VERSION}" /opt/zig; \
    ln -s /opt/zig/zig /usr/local/bin/zig

WORKDIR /map
COPY . .

# Generate a CC wrapper per architecture.
# The Makefile hands CC down to every dep as `$(MAKE) CC="$(CC)"` and the dep
# makefiles use it as a single command, so a wrapper keeps a multi-word
# toolchain such as "zig cc -target ..." simple to pass around.
RUN set -eux; \
    for spec in \
        "amd64 x86_64-linux-musl" \
        "arm64v8 aarch64-linux-musl" \
    ; do \
        name="${spec%% *}"; \
        target="${spec##* }"; \
        printf '#!/bin/sh\nexec zig cc -target %s -Os "$@"\n' "$target" > "/usr/local/bin/cc-$name"; \
        chmod +x "/usr/local/bin/cc-$name"; \
    done

RUN set -eux; \
    for a in amd64 arm64v8; do \
        make clean; \
        ARCH="$a" CC="cc-$a" make deps all STRIP=/usr/bin/strip; \
        mv "map-$a" /usr/local/bin; \
    done; \
    cd /usr/local/bin && file map-*