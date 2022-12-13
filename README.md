# The LLVM Compiler Infrastructure

This directory and its sub-directories contain the source code for LLVM,
a toolkit for the construction of highly optimized compilers,
optimizers, and run-time environments.

The README briefly describes how to get started with building LLVM.
For more information on how to contribute to the LLVM project, please
take a look at the
[Contributing to LLVM](https://llvm.org/docs/Contributing.html) guide.

## Getting Started with the LLVM System

Taken from [here](https://llvm.org/docs/GettingStarted.html).

### Overview

Welcome to the LLVM project!

The LLVM project has multiple components. The core of the project is
itself called "LLVM". This contains all of the tools, libraries, and header
files needed to process intermediate representations and convert them into
object files. Tools include an assembler, disassembler, bitcode analyzer, and
bitcode optimizer. It also contains basic regression tests.

C-like languages use the [Clang](http://clang.llvm.org/) frontend. This
component compiles C, C++, Objective-C, and Objective-C++ code into LLVM bitcode
-- and from there into object files, using LLVM.

Other components include:
the [libc++ C++ standard library](https://libcxx.llvm.org),
the [LLD linker](https://lld.llvm.org), and more.

### Getting the Source Code and Building LLVM

The LLVM Getting Started documentation may be out of date. The [Clang
Getting Started](http://clang.llvm.org/get_started.html) page might have more
accurate information.

This is an example work-flow and configuration to get and build the LLVM source:

1. Checkout LLVM (including related sub-projects like Clang):

     * ``git clone https://github.com/llvm/llvm-project.git``

     * Or, on windows, ``git clone --config core.autocrlf=false
    https://github.com/llvm/llvm-project.git``

2. Configure and build LLVM and Clang:

     * ``cd llvm-project``

     * ``cmake -S llvm -B build -G <generator> [options]``

        Some common build system generators are:

        * ``Ninja`` --- for generating [Ninja](https://ninja-build.org)
          build files. Most llvm developers use Ninja.
        * ``Unix Makefiles`` --- for generating make-compatible parallel makefiles.
        * ``Visual Studio`` --- for generating Visual Studio projects and
          solutions.
        * ``Xcode`` --- for generating Xcode projects.

        Some common options:

        * ``-DLLVM_ENABLE_PROJECTS='...'`` and ``-DLLVM_ENABLE_RUNTIMES='...'`` ---
          semicolon-separated list of the LLVM sub-projects and runtimes you'd like to
          additionally build. ``LLVM_ENABLE_PROJECTS`` can include any of: clang,
          clang-tools-extra, cross-project-tests, flang, libc, libclc, lld, lldb,
          mlir, openmp, polly, or pstl. ``LLVM_ENABLE_RUNTIMES`` can include any of
          libcxx, libcxxabi, libunwind, compiler-rt, libc or openmp. Some runtime
          projects can be specified either in ``LLVM_ENABLE_PROJECTS`` or in
          ``LLVM_ENABLE_RUNTIMES``.

          For example, to build LLVM, Clang, libcxx, and libcxxabi, use
          ``-DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi"``.

        * ``-DCMAKE_INSTALL_PREFIX=directory`` --- Specify for *directory* the full
          path name of where you want the LLVM tools and libraries to be installed
          (default ``/usr/local``). Be careful if you install runtime libraries: if
          your system uses those provided by LLVM (like libc++ or libc++abi), you
          must not overwrite your system's copy of those libraries, since that
          could render your system unusable. In general, using something like
          ``/usr`` is not advised, but ``/usr/local`` is fine.

        * ``-DCMAKE_BUILD_TYPE=type`` --- Valid options for *type* are Debug,
          Release, RelWithDebInfo, and MinSizeRel. Default is Debug.

        * ``-DLLVM_ENABLE_ASSERTIONS=On`` --- Compile with assertion checks enabled
          (default is Yes for Debug builds, No for all other build types).

      * ``cmake --build build [-- [options] <target>]`` or your build system specified above
        directly.

        * The default target (i.e. ``ninja`` or ``make``) will build all of LLVM.

        * The ``check-all`` target (i.e. ``ninja check-all``) will run the
          regression tests to ensure everything is in working order.

        * CMake will generate targets for each tool and library, and most
          LLVM sub-projects generate their own ``check-<project>`` target.

        * Running a serial build will be **slow**. To improve speed, try running a
          parallel build. That's done by default in Ninja; for ``make``, use the option
          ``-j NNN``, where ``NNN`` is the number of parallel jobs to run.
          In most cases, you get the best performance if you specify the number of CPU threads you have.
          On some Unix systems, you can specify this with ``-j$(nproc)``.

      * For more information see [CMake](https://llvm.org/docs/CMake.html).

Consult the
[Getting Started with LLVM](https://llvm.org/docs/GettingStarted.html#getting-started-with-llvm)
page for detailed information on configuring and compiling LLVM. You can visit
[Directory Layout](https://llvm.org/docs/GettingStarted.html#directory-layout)
to learn about the layout of the source code tree.

## Getting in touch

Join [LLVM Discourse forums](https://discourse.llvm.org/), [discord chat](https://discord.gg/xS7Z362) or #llvm IRC channel on [OFTC](https://oftc.net/).

The LLVM project has adopted a [code of conduct](https://llvm.org/docs/CodeOfConduct.html) for
participants to all modes of communication within the project.

# RTC instructions

The runtimecore branch of this project will include any custom checks and tools that are not part of the upstream of the
llvm-project. These changes cannot easily be fast-forwarded as they are generally based on a release of llvm which
diverges from the main trunk every release. Once RTC upgrades, a tag called runtimecore_X.X.X will be created
symbolicating the llvm-project release that was being used at the time.

RTC uses a few llvm tools that require custom fixes and custom checks that are not officially part of the llvm project.
This means that the tools need to be hand-built. Below are the minimum instructions needed to build these tools locally.

> NOTE: Upgrading clang-tidy will require checking all enabled checks against the new version as the names of checks can
change and will silently not run. This can easily be accomplished by dumping the configuration with the new version of
clang-tidy and checking it against the older configuration.

The linux clang compiler is also a custom build. This is due to the need of using libc++ and libc++abi instead of
libstdc++. Since some of our products link to libstdc++, it is crucial to statically link in these runtimes and to hide
all symbols in order to allow using both runtimes in one application.

## Building LLVM Tools for RTC

RTC builds the llvm tools in order to add custom clang-tidy checks and adds fixes to existing ones to work with our
codebase. This requires a workspace setup to get all the additional tools that we use for code quality.

```sh
mkdir ${HOME}/llvm && cd ${HOME}/llvm
git clone --branch runtimecore_15.0.4 git@github.com:Esri/llvm-project.git
git clone --branch runtimecore_15.0.4 git@github.com:Esri/include-what-you-use.git
```

### Linux

To build on Linux, you MUST use the [llvm.dockerfile](llvm.dockerfile) dockerfile to build. This will give you the
correct ubuntu 20.04 environment that is needed to build with a compatible ABI. This matches our lowest supported
platform for RTC and also makes sure that we're building with a clean environment C runtime. Using docker will also more
easily allow you to build for arm64 using an arm64 based mac machine to get good compilation times. To build the image,
you can run the following command:

`docker build --tag=llvm:15.0.4 - < llvm-project/llvm.dockerfile`

This will create the needed Ubuntu 20.04 sand-boxed environment that we'll use to build. This makes it so you can use
any version of Ubuntu but still get the correct artifacts. It also ensures a minimal build environment that won't
conflict with your host system. When you're ready to build, you can then start the container with the following command:

`docker run --rm -it -u $(id -u):$(id -g) --volume ${HOME}/llvm:/llvm --workdir /llvm llvm:15.0.4 bash`

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
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;lld" \
  -DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" \
  -DLLVM_USE_LINKER="lld" \
  \
  -DCLANG_ENABLE_BOOTSTRAP=On \
  \
  -DBOOTSTRAP_CMAKE_INSTALL_PREFIX="/llvm/15.0.4" \
  \
  -DBOOTSTRAP_LLVM_DEFAULT_TARGET_TRIPLE="$(uname -m)-unknown-linux-gnu" \
  -DBOOTSTRAP_LLVM_ENABLE_LTO="Thin" \
  -DBOOTSTRAP_LLVM_ENABLE_PER_TARGET_RUNTIME_DIR="OFF" \
  -DBOOTSTRAP_LLVM_EXTERNAL_IWYU_SOURCE_DIR="/llvm/include-what-you-use" \
  -DBOOTSTRAP_LLVM_EXTERNAL_PROJECTS="iwyu" \
  -DBOOTSTRAP_LLVM_INSTALL_TOOLCHAIN_ONLY="ON" \
  -DBOOTSTRAP_LLVM_USE_LINKER="lld" \
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

# run the clang tools tests to make sure the Esri specific tests pass
cmake --build build -- check-clang-tools

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

### macOS

To build on macOS, use RTC's well known compiler paths in order to keep equivalent behavior. The macOS configuration is
nearly identical from linux but builds universal binaries to work with multiple architectures. It also turns off zstd
support as it isn't needed and doesn't work with universal builds.

```sh
# Install dependencies
brew install cmake ccache ninja

# Set the Xcode version to RTC's Xcode_13.2.1
sudo xcode-select --switch /Applications/Xcode_13.2.1.app/Contents/Developer

# Configure the release build (Use Debug instead of Release in CMAKE_BUILD_TYPE to debug tools)
cmake -S llvm-project/llvm -B build -G "Ninja" \
  -DCMAKE_BUILD_TYPE="Release" \
  -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
  -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
  -DCMAKE_INSTALL_PREFIX="15.0.4" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  \
  -DLLVM_ENABLE_LTO="Thin" \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
  -DLLVM_ENABLE_RUNTIMES="libcxx" \
  -DLLVM_ENABLE_ZSTD="OFF" \
  -DLLVM_EXTERNAL_IWYU_SOURCE_DIR="include-what-you-use" \
  -DLLVM_EXTERNAL_PROJECTS="iwyu"

# build, check, and install the tools
cmake --build build -- check-clang-tools include-what-you-use
cmake --build build -- install-clang-format install-clang-resource-headers install-clang-tidy tools/iwyu/install

# build and install the v1 headers needed by clang-tidy. This step will fail to link but we only need the headers and
# there's no rule to only install the headers
cmake --build build -- install-cxx
```

### Windows

To build on Windows, you'll need to install Visual Studio 2022 to get access to a C++ compiler and the Developer
console. You'll also need to install [chocolatey](https://chocolatey.org/install) in order to easily install cmake and
ninja which will be used by the build. Once that is installed and choco is on the path, open an developer prompt by
navigating to `Start` -> `x64 Native Tools Command Prompt for VS 2022` to run the following steps:

```cmd
# Install dependencies
choco install cmake --installargs '"ADD_CMAKE_TO_PATH=System"'
choco install ninja

# Configure the release build (Use Debug instead of Release in CMAKE_BUILD_TYPE to debug tools)
cmake -S llvm-project/llvm -B build -G "Ninja" ^
  -DCMAKE_BUILD_TYPE="Release" ^
  -DCMAKE_INSTALL_PREFIX="15.0.4" ^
  ^
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" ^
  -DLLVM_EXTERNAL_IWYU_SOURCE_DIR="include-what-you-use" ^
  -DLLVM_EXTERNAL_PROJECTS="iwyu"

# build, check, and install the tools
cmake --build build -- check-clang-tools include-what-you-use
cmake --build build -- install-clang-format install-clang-resource-headers install-clang-tidy tools/iwyu/install
```

### Packaging

Once all tools are built and installed for a platform, you should have a 15.0.4 folder in your llvm folder. The final
step here will be to zip this package up and archive them onto our network shares. From here, they can be pulled by the
install_dependencies framework and placed locally on developer machines.

## Working with LLVM AST and Writing Your Own Clang-Tidy Checkers

The document [Working with the LLVM AST](working_with_the_llvm_ast.md) explores resources and tips for understanding
the LLVM AST, writing new clang-tidy checkers and modifying existing clang-tidy checkers.
