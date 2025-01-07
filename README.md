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

## Setup

To get setup for the build, clone the right sources to a user agnostic location:

```bash
LLVM_HOME="/c/llvm" # windows
if [[ "$(uname)" == *"Darwin"* ]]; then
  # set a user agnostic home and optionally create a HOME symlink to work more easily with the source
  LLVM_HOME="/Users/Shared/llvm"
  ln -s -f ${LLVM_HOME} ${HOME}/llvm

elif [[ "${os}" == *"Linux"* ]]; then
  # okay to clone directly into HOME since the source will be mounted into a user agnostic location into the container
  LLVM_HOME="${HOME}/llvm"
fi

mkdir -p ${LLVM_HOME} && cd ${LLVM_HOME}
git clone --branch runtimecore_19.1.2 git@github.com:Esri/include-what-you-use.git
git clone --branch runtimecore_19.1.2 git@github.com:Esri/llvm-project.git
```

## Linux

### Building the Linux Compiler

To build on Linux, you MUST use the [llvm.dockerfile](llvm.dockerfile) dockerfile to build. This will give you the
correct ubuntu 22.04 environment that is needed to build with a compatible ABI. This matches our lowest supported
platform for RTC and also makes sure that we're building with a clean environment C runtime. Using docker will also more
easily allow you to build for arm64 using an arm64 based mac machine to get good compilation times. To build the images,
you can run the following commands:

```bash
docker build --platform=linux/amd64 --tag=llvm:19.1.2-amd64 - < ${HOME}/llvm/llvm-project/llvm.dockerfile
docker build --platform=linux/arm64 --tag=llvm:19.1.2-arm64 - < ${HOME}/llvm/llvm-project/llvm.dockerfile
```

This will create the needed Ubuntu 22.04 sand-boxed environments that we'll use to build. When you're ready to build,
you can then start the container with the following command:

`docker run --rm -it -u $(id -u):$(id -g) --volume ${HOME}/llvm:/llvm --workdir /llvm llvm:19.1.2-amd64 bash`

You'll now be in the container environment bash and can run the cmake commands to build. The following Cmake commands
are very similar to the [Fushsia](clang/cmake/caches/Fuchsia.cmake) commands. The commands will build the llvm compiler
as well as all the runtimes that will be statically linked into RTC so the library can be as portable as possible and
can run on applications that use the `libstdc++` runtime.

```bash
# ${target} is set at the dockerfile for convenience
cmake -S llvm-project/llvm -B /llvm/${target}/build -G "Ninja" \
  -DCMAKE_AR="/usr/bin/llvm-ar" \
  -DCMAKE_BUILD_TYPE="Release" \
  -DCMAKE_C_COMPILER="/usr/bin/clang" \
  -DCMAKE_C_COMPILER_LAUNCHER="/usr/bin/ccache" \
  -DCMAKE_CXX_COMPILER="/usr/bin/clang++" \
  -DCMAKE_CXX_COMPILER_LAUNCHER="/usr/bin/ccache" \
  -DCMAKE_INSTALL_PREFIX="/llvm/${target}/19.1.2" \
  -DCMAKE_RANLIB="/usr/bin/llvm-ranlib" \
  \
  -DIWYU_RESOURCE_RELATIVE_TO="iwyu" \
  \
  -DLLVM_BUILTIN_TARGETS="${target}" \
  -DLLVM_ENABLE_LTO="Thin" \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;lld" \
  -DLLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" \
  -DLLVM_EXTERNAL_IWYU_SOURCE_DIR="/llvm/include-what-you-use" \
  -DLLVM_EXTERNAL_PROJECTS="iwyu" \
  -DLLVM_RUNTIME_TARGETS="${target}" \
  -DLLVM_USE_LINKER="lld" \
  \
  -DRUNTIMES_${target}_CMAKE_SYSTEM_NAME="Linux" \
  -DRUNTIMES_${target}_CMAKE_BUILD_TYPE="Release" \
  -DRUNTIMES_${target}_COMPILER_RT_USE_BUILTINS_LIBRARY="ON" \
  -DRUNTIMES_${target}_LIBUNWIND_ENABLE_SHARED="OFF" \
  -DRUNTIMES_${target}_LIBUNWIND_USE_COMPILER_RT="ON" \
  -DRUNTIMES_${target}_LIBUNWIND_INSTALL_LIBRARY="OFF" \
  -DRUNTIMES_${target}_LIBCXXABI_USE_COMPILER_RT="ON" \
  -DRUNTIMES_${target}_LIBCXXABI_ENABLE_SHARED="OFF" \
  -DRUNTIMES_${target}_LIBCXXABI_USE_LLVM_UNWINDER="ON" \
  -DRUNTIMES_${target}_LIBCXXABI_ENABLE_STATIC_UNWINDER="ON" \
  -DRUNTIMES_${target}_LIBCXXABI_INSTALL_LIBRARY="OFF" \
  -DRUNTIMES_${target}_LIBCXX_USE_COMPILER_RT="ON" \
  -DRUNTIMES_${target}_LIBCXX_ENABLE_SHARED="OFF" \
  -DRUNTIMES_${target}_LIBCXX_ENABLE_STATIC_ABI_LIBRARY="ON" \
  -DRUNTIMES_${target}_LIBCXX_ABI_VERSION="2" \
  -DRUNTIMES_${target}_LLVM_ENABLE_ASSERTIONS="OFF" \
  -DRUNTIMES_${target}_LLVM_ENABLE_RUNTIMES="compiler-rt;libcxx;libcxxabi;libunwind" \
  -DRUNTIMES_${target}_SANITIZER_CXX_ABI="libc++" \
  -DRUNTIMES_${target}_SANITIZER_CXX_ABI_INTREE="ON"

# run the clang tools tests to make sure the Esri specific tests pass
cmake --build /llvm/${target}/build -- check-clang-tools

# install
cmake --build /llvm/${target}/build -- install/strip
```

Once you're done with the build for x64, exit the container and perform the above step again but this time, using the
arm64 container with:

`docker run --rm -it -u $(id -u):$(id -g) --volume ${HOME}/llvm:/llvm --workdir /llvm llvm:19.1.2-arm64 bash`

### Packaging the Linux Compiler

At this point, you'll have two toolchains that are built for x86_64 and aarch64. LLVM does provide support for
cross-compiling toolchains using bootstrapping but doing so requires setting up sysroots and cmake files that would be
good to check out during the next iteration of RTC's compiler upgrade but to move forward, instead just build
these toolchains independently and merge any new files from both architectures into each other for simplicity. When
the toolchains are zipped, add -x86_64 or -aarch64 to the package name so the install dependencies framework can get
the right architecture for the target node by using the ansibile_architecure.

To merge the folders, you can perform a diff and then move over folders from one to the other and vice-versa.

```bash
$ diff -r aarch64-unknown-linux-gnu/19.1.2 x86_64-unknown-linux-gnu/19.1.2/ | grep -v 'Binary files'
Only in aarch64-unknown-linux-gnu/19.1.2/include: aarch64-unknown-linux-gnu
Only in x86_64-unknown-linux-gnu/19.1.2/include: x86_64-unknown-linux-gnu
Only in aarch64-unknown-linux-gnu/19.1.2/lib: aarch64-unknown-linux-gnu
Only in aarch64-unknown-linux-gnu/19.1.2/lib/clang/19/lib: aarch64-unknown-linux-gnu
Only in x86_64-unknown-linux-gnu/19.1.2/lib/clang/19/lib: x86_64-unknown-linux-gnu
Only in x86_64-unknown-linux-gnu/19.1.2/lib: x86_64-unknown-linux-gnu

cp -r aarch64-unknown-linux-gnu/19.1.2/include/aarch64-unknown-linux-gnu x86_64-unknown-linux-gnu/19.1.2/include/
cp -r x86_64-unknown-linux-gnu/19.1.2/include/x86_64-unknown-linux-gnu aarch64-unknown-linux-gnu/19.1.2/include/
cp -r aarch64-unknown-linux-gnu/19.1.2/lib/aarch64-unknown-linux-gnu x86_64-unknown-linux-gnu/19.1.2/lib/
cp -r x86_64-unknown-linux-gnu/19.1.2/lib/x86_64-unknown-linux-gnu aarch64-unknown-linux-gnu/19.1.2/lib/
cp -r aarch64-unknown-linux-gnu/19.1.2/lib/clang/19/lib/aarch64-unknown-linux-gnu x86_64-unknown-linux-gnu/19.1.2/lib/clang/19/lib/
cp -r x86_64-unknown-linux-gnu/19.1.2/lib/clang/19/lib/x86_64-unknown-linux-gnu aarch64-unknown-linux-gnu/19.1.2/lib/clang/19/lib/

$ diff -r aarch64-unknown-linux-gnu/19.1.2 x86_64-unknown-linux-gnu/19.1.2/ | grep -v 'Binary files'
# no output means we're synced
```

Now that both folders have the missing bits for both, zip them up using the architecture names so both cross-compilers
can have the native binaries.

```bash
cd ${HOME}/llvm/x86_64-unknown-linux-gnu && zip -r llvm-19.1.2-x86_64.zip 19.1.2
cd ${HOME}/llvm/aarch64-unknown-linux-gnu && zip -r llvm-19.1.2-aarch64.zip 19.1.2
```

## macOS

To build on macOS, use RTC's well known compiler paths in order to keep equivalent behavior. The macOS configuration is
nearly identical from linux but builds universal binaries to work with multiple architectures. It also turns off zstd
support as it isn't needed and doesn't work with universal builds.

```bash
# Install dependencies
brew install cmake ccache ninja

# Set the Xcode version to RTC's version
sudo xcode-select --switch /Applications/Xcode_15.2.0.app/Contents/Developer

# Configure the release build (Use Debug instead of Release in CMAKE_BUILD_TYPE to debug tools)
cmake -S /Users/Shared/llvm/llvm-project/llvm -B /Users/Shared/llvm/build -G "Ninja" \
  -DCMAKE_BUILD_TYPE="Release" \
  -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
  -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
  -DCMAKE_INSTALL_PREFIX="${HOME}/llvm/19.1.2" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  \
  -DIWYU_RESOURCE_RELATIVE_TO="iwyu" \
  \
  -DLLVM_ENABLE_LTO="Thin" \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
  -DLLVM_ENABLE_ZSTD="OFF" \
  -DLLVM_EXTERNAL_IWYU_SOURCE_DIR="/Users/Shared/llvm/include-what-you-use" \
  -DLLVM_EXTERNAL_PROJECTS="iwyu"

# run the clang tools tests to make sure the Esri specific tests pass
cmake --build /Users/Shared/llvm/build -- check-clang-tools

# install
cmake --build /Users/Shared/llvm/build -- install-clang-apply-replacements-stripped install-clang-format-stripped install-clang-resource-headers-stripped install-clang-tidy-stripped install-cmake-exports-stripped tools/iwyu/install/strip

# zip
cd /Users/Shared/llvm && zip -r llvm-19.1.2.zip 19.1.2
```

## Windows

To build on Windows, you'll need to install Visual Studio 2022 to get access to a C++ compiler and the Developer
console. You'll also need to install [chocolatey](https://chocolatey.org/install) in order to easily install cmake and
ninja which will be used by the build. Once that is installed and choco is on the path, open an developer prompt by
navigating to `Start` -> `x64 Native Tools Command Prompt for VS 2022` to run the following steps:

```bat
:: Install dependencies
choco install cmake --installargs '"ADD_CMAKE_TO_PATH=System"'
choco install ninja

:: Configure the release build (Use Debug instead of Release in CMAKE_BUILD_TYPE to debug tools)
cmake -S C:/llvm/llvm-project/llvm -B C:/llvm/build -G "Ninja" ^
  -DCMAKE_BUILD_TYPE="Release" ^
  -DCMAKE_INSTALL_PREFIX="C:/llvm/19.1.2" ^
  ^
  -DIWYU_RESOURCE_RELATIVE_TO="iwyu" ^
  ^
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" ^
  -DLLVM_EXTERNAL_IWYU_SOURCE_DIR="C:/llvm/include-what-you-use" ^
  -DLLVM_EXTERNAL_PROJECTS="iwyu"

:: run the clang tools tests to make sure the Esri specific tests pass
cmake --build C:/llvm/build -- check-clang-tools

:: install
cmake --build C:/llvm/build -- install-clang-apply-replacements-stripped install-clang-format-stripped install-clang-resource-headers-stripped install-clang-tidy-stripped install-cmake-exports-stripped tools/iwyu/install

:: zip
powershell Compress-Archive C:/llvm/19.1.2 C:/llvm/llvm-19.1.2.zip
```

## Cleanup

The llvm builds takes a significant amount of space to clone, build, cache and install all the binaries. Once you're
done with the build, you may want to delete your llvm folder entirely if space is an issue.

## Working with LLVM AST and Writing Your Own Clang-Tidy Checkers

The document [Working with the LLVM AST](working_with_the_llvm_ast.md) explores resources and tips for understanding
the LLVM AST, writing new clang-tidy checkers and modifying existing clang-tidy checkers.

# The LLVM Compiler Infrastructure

[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/llvm/llvm-project/badge)](https://securityscorecards.dev/viewer/?uri=github.com/llvm/llvm-project)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/8273/badge)](https://www.bestpractices.dev/projects/8273)
[![libc++](https://github.com/llvm/llvm-project/actions/workflows/libcxx-build-and-test.yaml/badge.svg?branch=main&event=schedule)](https://github.com/llvm/llvm-project/actions/workflows/libcxx-build-and-test.yaml?query=event%3Aschedule)

Welcome to the LLVM project!

This repository contains the source code for LLVM, a toolkit for the
construction of highly optimized compilers, optimizers, and run-time
environments.

The LLVM project has multiple components. The core of the project is
itself called "LLVM". This contains all of the tools, libraries, and header
files needed to process intermediate representations and convert them into
object files. Tools include an assembler, disassembler, bitcode analyzer, and
bitcode optimizer.

C-like languages use the [Clang](https://clang.llvm.org/) frontend. This
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
chat](https://discord.gg/xS7Z362),
[LLVM Office Hours](https://llvm.org/docs/GettingInvolved.html#office-hours) or
[Regular sync-ups](https://llvm.org/docs/GettingInvolved.html#online-sync-ups).

The LLVM project has adopted a [code of conduct](https://llvm.org/docs/CodeOfConduct.html) for
participants to all modes of communication within the project.
