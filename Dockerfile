# 使用 zig 自带 musl 交叉编译，构建 2 个架构的静态 musl 二进制
#
# 架构对照（zig 目标名）：
#   amd64   -> x86_64-linux-musl
#   arm64v8 -> aarch64-linux-musl
#
# 注意：本 Docker 的 RUN 原样传给 /bin/sh（不做 $$ 转义），
#       所以这里直接用 $VAR / $(...)，不要写成 $$。
FROM debian:trixie-slim

ENV ZIG_VERSION=0.16.0

# 工具依赖：
#   binutils-multiarch —— 默认 binutils 的 strip 只认宿主附近架构，
#                         需要 multiarch 版才能 strip 任意架构的 ELF
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

# 为每个架构生成 CC 包装脚本。
# 项目 Makefile 的 deps 规则里是 `CC=$(CC) $(MAKE)`，要求 CC 是单条命令，
# 因此不能直接用 "zig cc -target ..."（含空格），用 wrapper 脚本最稳。
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