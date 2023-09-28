# Container that has all the tools needed to build the llvm-project and all additional tools
FROM ubuntu:20.04

# This stops krb5-user package from prompting for geographic region
ENV DEBIAN_FRONTEND=noninteractive

# install dependencies
RUN \
  apt update \
  && \
  apt install -y \
  ccache \
  clang \
  curl \
  git \
  lld \
  llvm \
  ninja-build \
  zlib1g-dev \
  && \
  rm -rf /var/lib/apt/lists/* /tmp/* \
  && \
  echo "Done"

# set the ccache dir to the llvm mount location so it can persist across containers
ENV CCACHE_DIR=/llvm/ccache

# install a later version of cmake needed to build
ENV cmake_version=3.20.0
RUN \
  curl --location --output /tmp/cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v${cmake_version}/cmake-${cmake_version}-linux-$(uname -m).tar.gz \
  && \
  tar xzvf /tmp/cmake.tar.gz --directory /usr/local --strip-components=1 \
  && \
  rm /tmp/cmake.tar.gz
