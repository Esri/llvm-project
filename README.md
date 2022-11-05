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

## RTC instructions

The runtimecore branch of this project will include any custom checks and tools that are not part of the upstream of the
llvm-project. These changes cannot easily be fast-forwarded as they are generally based on a release of llvm which
diverges from the main trunk every release. Once RTC upgrades, a tag called runtimecore_X.X.X will be created
symbolicating the llvm-project release that was being used at the time.

RTC uses a few llvm tools that require custom fixes and custom checks that are not officially part of the llvm project.
This means that the tools need to be hand-built. Below are the minimum instructions needed to build these tools locally.

> NOTE: Upgrading clang-tidy will require checking all enabled checks against the new version as the names of checks can
change and will silently not run. This can easily be accomplished by dumping the configuration with the new version of
clang-tidy and checking it against the older configuration.

### Clone the runtimecore branch of the llvm-project to get access to the esri additional fixes and checks

Clone the custom branch of llvm and create a build directory where we'll create the build and store all intermediate and
build files.

```sh
git clone -b runtimecore https://github.com/Esri/llvm-project.git ${HOME}
mkdir -p ${HOME}/llvm-project/build && cd llvm-project/build
```

## Linux

Building the tools on Linux assumes that the initial build of linux below has already been completed and can be used
here to bootstrap the process. The initial build below lists all steps needed to create the custom compiler but for the
llvm tools, you do not need to go through with the entire process as the special considerations are only required for
the build server.

```sh
# Install dependencies
sudo apt-get install cmake ccache wget
wget http://runtimezip.esri.com:8080/userContent/apps-archive/archive/local_system_setup/runtimecore/linux/9.0.0_clang_libc++_x64.tar.gz
sudo tar xzPf 9.0.0_clang_libc++_x64.tar.gz
rm 9.0.0_clang_libc++_x64.tar.gz
# Release
cmake -G "Unix Makefiles" ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;" \
  -DCMAKE_INSTALL_PREFIX=${HOME}/rtc/llvm/9.0.0 \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_LTO=Thin \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_C_COMPILER=/usr/local/rtc/llvm/9.0.0/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/local/rtc/llvm/9.0.0/bin/clang++ \
  -DCMAKE_AR=/usr/local/rtc/llvm/9.0.0/bin/llvm-ar \
  -DCMAKE_LINKER=/usr/local/rtc/llvm/9.0.0/bin/lld \
  -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld -Wl,--thinlto-cache-dir=${HOME}/.thinLTO -Wl,--icf=all" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld -Wl,--thinlto-cache-dir=${HOME}/.thinLTO -Wl,--icf=all" \
  -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld -Wl,--thinlto-cache-dir=${HOME}/.thinLTO -Wl,--icf=all" \
  -DIWYU_IN_TREE=ON \
  -DCONSTEXPR_EVERYTHING_IN_TREE=ON
# Debug
cmake -G "Unix Makefiles" ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;" \
  -DCMAKE_INSTALL_PREFIX=${HOME}/rtc/llvm/9.0.0 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_C_COMPILER=/usr/local/rtc/llvm/9.0.0/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/local/rtc/llvm/9.0.0/bin/clang++ \
  -DCMAKE_RANLIB=/usr/local/rtc/llvm/9.0.0/bin/llvm-ranlib \
  -DCMAKE_AR=/usr/local/rtc/llvm/9.0.0/bin/llvm-ar \
  -DCMAKE_LINKER=/usr/local/rtc/llvm/9.0.0/bin/lld \
  -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
  -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld" \
  -DIWYU_IN_TREE=ON \
  -DCONSTEXPR_EVERYTHING_IN_TREE=ON
make -j10 -k check-clang-format check-clang-tools constexpr-everything include-what-you-use templight
make -j10 install-clang-format install-clang-tidy install-constexpr-everything install-include-what-you-use install-templight
```

Since these tools will be built somewhat regularly with new features, unlike the rest of the compiler, it is best to
copy the include folder from /usr/local/rtc/llvm/9.0.0 and put it into ${HOME}/rtc/llvm/9.0.0 to get the right headers.

## macOS

```sh
brew install cmake ccache
# Release
cmake -G "Unix Makefiles" ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;" \
  -DCMAKE_INSTALL_PREFIX=${HOME}/rtc/llvm/9.0.0 \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_LTO=Thin \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,-cache_path_lto,${HOME}/.thinLTO -Wl,-prune_after_lto,604800" \
  -DCMAKE_SHARED_LINKER_FLAGS="-Wl,-cache_path_lto,${HOME}/.thinLTO -Wl,-prune_after_lto,604800" \
  -DCMAKE_MODULE_LINKER_FLAGS="-Wl,-cache_path_lto,${HOME}/.thinLTO -Wl,-prune_after_lto,604800" \
  -DIWYU_IN_TREE=ON \
  -DCONSTEXPR_EVERYTHING_IN_TREE=ON
# Debug
cmake -G "Unix Makefiles" ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;" \
  -DCMAKE_INSTALL_PREFIX=${HOME}/rtc/llvm/9.0.0 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DIWYU_IN_TREE=ON \
  -DCONSTEXPR_EVERYTHING_IN_TREE=ON
make -j10 -k check-clang-format check-clang-tools constexpr-everything include-what-you-use templight
make -j10 install-clang-format install-clang-tidy install-constexpr-everything install-include-what-you-use install-templight
```

## Windows

```sh
# Release
cmake -G "Visual Studio 16 2019" ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
  -DCMAKE_BUILD_TYPE=Release \
  -Thost=x64 \
  -DIWYU_IN_TREE=ON \
  -DCONSTEXPR_EVERYTHING_IN_TREE=ON
# Debug
cmake -G "Visual Studio 16 2019" ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
  -DCMAKE_BUILD_TYPE=Debug \
  -Thost=x64 \
  -DIWYU_IN_TREE=ON \
  -DCONSTEXPR_EVERYTHING_IN_TREE=ON
# Open LLVM.sln and build check-clang-tools
# Binaries will be in Release/bin. Most of the tests won't work as they're really only tuned for linux
```

### Linux Initial Build with RHEL 7

For linux, we also need to hand build clang and libc++ as a method to be able to use C++17 which is not possible when
using libstdc++. This is because the lowest support platform that we support does not have that capability. We'll need
to build everything on our lowest supported platform which is RHEL 7, devtoolset 4. This contains the oldest libstdc++
and libc (GLIBC 2.17). Builds against these libs are forward compatible but not backwards compatible. Make sure you have
enough room on the VM (75GB).

#### Install a few dependencies

```sh
sudo yum install git wget
# Follow https://www.softwarecollections.org/en/scls/rhscl/devtoolset-4/ to install devtools-4 for a newer libstdc++
# Follow http://jotmynotes.blogspot.com/2016/10/updating-cmake-from-2811-to-362-or.html to install cmake but use a
# version higher than 3.8.
# Follow https://snapcraft.io/install/ccache/centos to install ccache
```

#### Download llvm 11 for ubuntu 16 to build our custom llvm 11 libraries and put it at /usr/local/llvm

```sh
wget https://releases.llvm.org/8.0.0/clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
tar -xf clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
sudo mv clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-14.04 /usr/local/llvm
rm clang+llvm-8.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
```

#### Build the clang toolchain and libraries specially in order to support RHEL

```sh
mkdir -p ~/llvm-project/build && cd ~/llvm-project/build

cmake -G "Unix Makefiles" ../llvm \
  -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;libcxx;libcxxabi;libunwind;lld" \
  -DCMAKE_INSTALL_PREFIX=/usr/local/rtc/llvm/11.0.0 \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_LTO=Thin \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_C_COMPILER=/usr/local/llvm/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/local/llvm/bin/clang++ \
  -DCMAKE_RANLIB=/usr/local/llvm/bin/llvm-ranlib \
  -DCMAKE_AR=/usr/local/llvm/bin/llvm-ar \
  -DCMAKE_NM=/usr/local/llvm/bin/llvm-nm \
  -DCMAKE_LINKER=/usr/local/llvm/bin/lld \
  -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld -Wl,--thinlto-cache-dir=${HOME}/.thinLTO -Wl,--icf=all" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld -Wl,--thinlto-cache-dir=${HOME}/.thinLTO -Wl,--icf=all" \
  -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld -Wl,--thinlto-cache-dir=${HOME}/.thinLTO -Wl,--icf=all" \
  \
  -DCOMPILER_RT_BUILD_XRAY=OFF \
  -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
  -DCOMPILER_RT_BUILD_PROFILE=ON \
  \
  -DLIBCXX_ENABLE_SHARED=OFF \
  -DLIBCXX_HERMETIC_STATIC_LIBRARY=ON \
  -DLIBCXX_USE_COMPILER_RT=ON \
  -DLIBCXX_CXX_ABI=libcxxabi \
  -DLIBCXX_CXX_ABI_INCLUDE_PATHS="${HOME}/llvm-project/libcxxabi/include" \
  -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON \
  \
  -DLIBCXXABI_ENABLE_SHARED=OFF \
  -DLIBCXXABI_HERMETIC_STATIC_LIBRARY=ON \
  -DLIBCXXABI_USE_COMPILER_RT=ON \
  -DLIBCXXABI_ENABLE_STATIC_UNWINDER=ON \
  -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
  \
  -DLIBUNWIND_ENABLE_SHARED=OFF \
  -DLIBUNWIND_USE_COMPILER_RT=ON \
  -DLIBUNWIND_HERMETIC_STATIC_LIBRARY=ON

make -j10 -k
sudo make install
```

Now tar these into a package that will be untarred onto developer's machines with only the bare minimum needed to build

```sh
tar --create --file=11.0.0_clang_libc++_x64.tar.gz --absolute-names -v --gzip /usr/local/rtc/llvm/11.0.0
```

## Working with LLVM AST and Writing Your Own Clang-Tidy Checkers

The document [Working with the LLVM AST](working_with_the_llvm_ast.md) explores resources and tips for understanding
the LLVM AST, writing new clang-tidy checkers and modifying existing clang-tidy checkers.
