//===--- ImplicitCastToSizetCheck.h - clang-tidy ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_ESRI_IMPLICITCASTTOSIZETCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_ESRI_IMPLICITCASTTOSIZETCHECK_H

#include "../ClangTidyCheck.h"

namespace clang {
namespace tidy {
namespace esri {

/// MSVC defines size_t as a 32-bit unsigned integer in x86 architectures but
/// not in x64, which means problems converting other integer types to size_t
/// often go unnoticed during the PR and vTest process.
///
/// This check is designed to highlight those problems before they make it into
/// the codebase and break that configuration of the daily build.
class ImplicitCastToSizetCheck : public ClangTidyCheck {
public:
  ImplicitCastToSizetCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace esri
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_ESRI_IMPLICITCASTTOSIZETCHECK_H
