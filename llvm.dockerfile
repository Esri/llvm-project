# Container that has all the tools needed to build the llvm-project and all additional tools
FROM ubuntu:22.04

# This stops krb5-user package from prompting for geographic region
ENV DEBIAN_FRONTEND="noninteractive"

# install dependencies
RUN \
  apt update \
  && \
  apt install -y \
  ccache \
  clang \
  cmake \
  curl \
  git \
  lld \
  llvm \
  ninja-build \
  zip \
  zlib1g-dev \
  && \
  rm -rf /var/lib/apt/lists/* /tmp/* \
  && \
  echo "Done"

# set the ccache dir to the llvm mount location so it can persist across containers
ENV CCACHE_DIR="/llvm/ccache"

# use docker build variables to set the llvm target architecture so all build commands can use it by using the build
# time TARGETARCH variable and manipulating it to get the right uname
ARG TARGETARCH
ENV TARGETARCH="${TARGETARCH/amd/x86_}"
ENV TARGETARCH="${TARGETARCH/arm/aarch}"
ENV target="${TARGETARCH}-unknown-linux-gnu"
