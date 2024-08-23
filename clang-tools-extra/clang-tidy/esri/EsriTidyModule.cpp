//===--- EsriTidyModule.cpp - clang-tidy ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Heavily borrowed from the MiscTidyModule.cpp
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "NoImplementationInHeadersCheck.h"

namespace clang {
namespace tidy {
namespace esri {

class EsriModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<NoImplementationInHeadersCheck>(
        "esri-no-implementation-in-headers");
  }
};

} // namespace esri

// Register the EsriTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<esri::EsriModule>
    X("esri-module", "Adds esri specific checks.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the EsriModule.
volatile int EsriModuleAnchorSource = 0;

} // namespace tidy
} // namespace clang
