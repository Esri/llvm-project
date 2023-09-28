# RTC instructions

The runtimecore branch of this project will include any custom checks and tools that are not part of the upstream
llvm-project. These changes cannot easily be fast-forwarded as they are generally based on a release of llvm which
diverges from the main trunk every release. Once RTC upgrades, a branch called runtimecore_X.X.X will be created
symbolicating the llvm-project release that was being used at the time. Sometimes an upgrade will only comprise the
linux compiler used. Other branches may be for the the llvm tools which are needed on all platforms. It is best to check
all the different runtimecore branches.

The linux clang compiler is also a custom build. This is due to the need of using libc++ and libc++abi instead of
libstdc++. Since some of our products link to libstdc++, it is crucial to statically link in these runtimes and to hide
all symbols in order to allow using both runtimes in one application.

To get setup for the build, clone the right sources:

```sh
mkdir ${HOME}/llvm && cd ${HOME}/llvm
git clone --branch runtimecore_17.0.1 git@github.com:Esri/llvm-project.git
git clone --branch runtimecore_17.0.1 git@github.com:Esri/include-what-you-use.git
```

## Building the Linux Compiler

To build on Linux, you MUST use the [llvm.dockerfile](llvm.dockerfile) dockerfile to build. This will give you the
correct ubuntu 20.04 environment that is needed to build with a compatible ABI. This matches our lowest supported
platform for RTC and also makes sure that we're building with a clean environment C runtime. Using docker will also more
easily allow you to build for arm64 using an arm64 based mac machine to get good compilation times. To build the image,
you can run the following command:

`docker build --tag=llvm:17.0.1 - < llvm-project/llvm.dockerfile`

This will create the needed Ubuntu 20.04 sand-boxed environment that we'll use to build. This makes it so you can use
any version of Ubuntu but still get the correct artifacts. It also ensures a minimal build environment that won't
conflict with your host system. When you're ready to build, you can then start the container with the following command:

`docker run --rm -it -u $(id -u):$(id -g) --volume ${HOME}/llvm:/llvm --workdir /llvm llvm:17.0.1 bash`

You'll now be in the container environment bash and can run the cmake commands to build.

```sh
# Configure the release build (Use Debug instead of Release in CMAKE_BUILD_TYPE to debug tools). Note that many of the
# options need to be passed through the bootstrap build and are prepended with BOOTSTRAP_. The options that don't have
# BOOSTRAP_ are either already defaulted as passthroughs using the BOOTSTRAP_DEFAULT_PASSTHROUGH list or changed by the
# bootstrap build already, such as the CXX compiler.
cmake -S llvm-project/llvm -B build -G "Ninja" \
  -DCMAKE_AR="/usr/bin/llvm-ar" \
  -DCMAKE_BUILD_TYPE="Release" \
  -DCMAKE_C_COMPILER="/usr/bin/clang" \
  -DCMAKE_C_COMPILER_LAUNCHER="/usr/bin/ccache" \
  -DCMAKE_CXX_COMPILER="/usr/bin/clang++" \
  -DCMAKE_CXX_COMPILER_LAUNCHER="/usr/bin/ccache" \
  -DCMAKE_RANLIB="/usr/bin/llvm-ranlib" \
  \
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" \
  -DLLVM_USE_LINKER="lld" \
  \
  -DCLANG_ENABLE_BOOTSTRAP=On \
  \
  -DBOOTSTRAP_CMAKE_INSTALL_PREFIX="/llvm/17.0.1" \
  \
  -DBOOTSTRAP_LLVM_ENABLE_LTO="Thin" \
  -DBOOTSTRAP_LLVM_INSTALL_TOOLCHAIN_ONLY="ON" \
  -DBOOTSTRAP_LLVM_USE_LINKER="lld" \
  \
  -DBOOTSTRAP_COMPILER_RT_CXX_LIBRARY="libcxx" \
  -DBOOTSTRAP_COMPILER_RT_USE_BUILTINS_LIBRARY="ON" \
  -DBOOTSTRAP_COMPILER_RT_USE_LLVM_UNWINDER="ON" \
  \
  -DBOOTSTRAP_LIBCXX_CXX_ABI="libcxxabi" \
  -DBOOTSTRAP_LIBCXX_ENABLE_SHARED="OFF" \
  -DBOOTSTRAP_LIBCXX_ENABLE_STATIC_ABI_LIBRARY="ON" \
  -DBOOTSTRAP_LIBCXX_HERMETIC_STATIC_LIBRARY="ON" \
  -DBOOTSTRAP_LIBCXX_USE_COMPILER_RT="ON" \
  \
  -DBOOTSTRAP_LIBCXXABI_ENABLE_SHARED="OFF" \
  -DBOOTSTRAP_LIBCXXABI_ENABLE_STATIC_UNWINDER="ON" \
  -DBOOTSTRAP_LIBCXXABI_HERMETIC_STATIC_LIBRARY="ON" \
  -DBOOTSTRAP_LIBCXXABI_USE_COMPILER_RT="ON" \
  -DBOOTSTRAP_LIBCXXABI_USE_LLVM_UNWINDER="ON" \
  \
  -DBOOTSTRAP_LIBUNWIND_ENABLE_SHARED="OFF" \
  -DBOOTSTRAP_LIBUNWIND_HIDE_SYMBOLS="ON" \
  -DBOOTSTRAP_LIBUNWIND_USE_COMPILER_RT="ON"

# build clang and then bootstrap the build with that clang to build all tools, runtimes and clang again using the
# bootstrap build technique documented at https://llvm.org/docs/AdvancedBuilds.html
cmake --build build -- stage2-install

# At this point, you'll have a toolchain that is built for x86_64 or aarch64. LLVM does provide support for
# cross-compiling toolchains using bootstrapping but doing so requires setting up sysroots and cmake files that would be
# good to check out during the next iteration of RTC's compiler upgrade but to move forward, instead just build
# these toolchains independently and merge any new files from both architectures into each other for simplicity. When
# the toolchains are zipped, add -x86_64 or -aarch64 to the package name so the install dependencies framework can get
# the right architecture for the target node by using the ansibile_architecure.
#
# The following are some helpful links on what was used to get to this point:
#
# https://github.com/llvm/llvm-project/issues/57104
# https://github.com/llvm/llvm-project/blob/main/clang/cmake/caches/Fuchsia-stage2.cmake
# https://mcilloni.ovh/2021/02/09/cxx-cross-clang/
```

### Packaging

Once all tools are built and installed for a platform, you should have a 17.0.1 folder in your llvm folder. The final
step here will be to zip this package up and archive them onto our network shares. From here, they can be pulled by the
install_dependencies framework and placed locally on developer machines.

# The LLVM Compiler Infrastructure

Welcome to the LLVM project!

This repository contains the source code for LLVM, a toolkit for the
construction of highly optimized compilers, optimizers, and run-time
environments.

The LLVM project has multiple components. The core of the project is
itself called "LLVM". This contains all of the tools, libraries, and header
files needed to process intermediate representations and convert them into
object files. Tools include an assembler, disassembler, bitcode analyzer, and
bitcode optimizer.

C-like languages use the [Clang](http://clang.llvm.org/) frontend. This
component compiles C, C++, Objective-C, and Objective-C++ code into LLVM bitcode
-- and from there into object files, using LLVM.

Other components include:
the [libc++ C++ standard library](https://libcxx.llvm.org),
the [LLD linker](https://lld.llvm.org), and more.

## Getting the Source Code and Building LLVM

Consult the
[Getting Started with LLVM](https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm)
page for information on building and running LLVM.

For information on how to contribute to the LLVM project, please take a look at
the [Contributing to LLVM](https://llvm.org/docs/Contributing.html) guide.

## Getting in touch

Join the [LLVM Discourse forums](https://discourse.llvm.org/), [Discord
chat](https://discord.gg/xS7Z362), or #llvm IRC channel on
[OFTC](https://oftc.net/).

The LLVM project has adopted a [code of conduct](https://llvm.org/docs/CodeOfConduct.html) for
participants to all modes of communication within the project.
