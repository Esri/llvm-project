# Container that has all the tools needed to build the llvm-project and all additional tools
FROM harbor-west.esri.com/runtime-docker-public/ubuntu:20.04

# This stops krb5-user package from prompting for geographic region
ENV DEBIAN_FRONTEND=noninteractive

# install dependencies
RUN \
  apt update \
  && \
  apt install -y \
  ccache \
  clang \
  cmake \
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
