//===--- NoImplementationInHeadersCheck.cpp - clang-tidy-------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file is custom modified to fit the ESRI coding guidelines.  It is
// heavily based upon the misc-definitions-in-headers file provided by clang.
//
//===----------------------------------------------------------------------===//

#include "NoImplementationInHeadersCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace esri {

namespace {

AST_MATCHER_P(NamedDecl, usesHeaderFileExtension,
              utils::HeaderFileExtensionsSet, HeaderFileExtensions) {
  return utils::isExpansionLocInHeaderFile(
      Node.getBeginLoc(), Finder->getASTContext().getSourceManager(),
      HeaderFileExtensions);
}

} // namespace

NoImplementationInHeadersCheck::NoImplementationInHeadersCheck(
    StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      UseHeaderFileExtension(Options.get("UseHeaderFileExtension", true)),
      AllowSoleDefaultDtor(Options.get("AllowSoleDefaultDtor", true)),
      RawStringHeaderFileExtensions(
          Options.getLocalOrGlobal("HeaderFileExtensions", ",h,hh,hpp,hxx")) {
  if (!utils::parseHeaderFileExtensions(RawStringHeaderFileExtensions,
                                        HeaderFileExtensions, ',')) {
    // FIXME: Find a more suitable way to handle invalid configuration
    // options.
    llvm::errs() << "Invalid header file extension: "
                 << RawStringHeaderFileExtensions << "\n";
  }
}

void NoImplementationInHeadersCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "UseHeaderFileExtension", UseHeaderFileExtension);
  Options.store(Opts, "HeaderFileExtensions", RawStringHeaderFileExtensions);
  Options.store(Opts, "AllowSoleDefaultDtor", AllowSoleDefaultDtor);
}

void NoImplementationInHeadersCheck::registerMatchers(MatchFinder *Finder) {
  if (!getLangOpts().CPlusPlus)
    return;
  auto DefinitionMatcher = anyOf(
      functionDecl(isDefinition(), unless(isDeleted()), unless(isImplicit())),
      varDecl(isDefinition()));
  if (UseHeaderFileExtension) {
    Finder->addMatcher(namedDecl(DefinitionMatcher,
                                 usesHeaderFileExtension(HeaderFileExtensions))
                           .bind("name-decl"),
                       this);
  } else {
    Finder->addMatcher(
        namedDecl(DefinitionMatcher,
                  anyOf(usesHeaderFileExtension(HeaderFileExtensions),
                        unless(isExpansionInMainFile())))
            .bind("name-decl"),
        this);
  }
}

void NoImplementationInHeadersCheck::check(
    const MatchFinder::MatchResult &Result) {
  // Don't run the check in failing TUs.
  if (Result.Context->getDiagnostics().hasUncompilableErrorOccurred())
    return;

  // C++ [basic.def.odr] p6:
  // There can be more than one definition of a class type, enumeration type,
  // inline function with external linkage, class template, non-static function
  // template, static data member of a class template, member function of a
  // class template, or template specialization for which some template
  // parameters are not specified in a program provided that each definition
  // appears in a different translation unit, and provided the definitions
  // satisfy the following requirements.
  const auto *ND = Result.Nodes.getNodeAs<NamedDecl>("name-decl");
  assert(ND);

  if (ND->isInvalidDecl())
    return;

  // Internal linkage variable definitions are ignored for now:
  //   const int a = 1;
  //   static int b = 1;
  //
  // Although these might also cause ODR violations, we can be less certain and
  // should try to keep the false-positive rate down.
  if(ND->isInAnonymousNamespace()) {
    diag(ND->getLocation(),
        "anonymously namespaced %0 defined in a header file; "
        "runtimecore prohibits anonymous namespaces in headers")
      << ND;
    return;
  }

  if (ND->getLinkageInternal() == InternalLinkage)
    return;

  if (const auto *FD = dyn_cast<FunctionDecl>(ND)) {
    if (!FD->isThisDeclarationADefinition()) {
      return;
    }

    // Function templates are allowed. Member function of a class template and
    // member function of a nested class in a class template are also allowed.
    if (FD->getTemplatedKind() == FunctionDecl::TK_FunctionTemplate) {
      return;
    }

    // Explicit specializations are prohibited in headers.
    if (FD->getTemplateSpecializationKind() == TSK_ExplicitSpecialization) {
      diag(FD->getLocation(),
           "explicit template specialization %0 defined in a header file; "
           "runtimecore prohibits explicit template specializations in headers"
           " and should be in the source file and externed in the header")
          << FD;
      return;
    }

    // Function templates instantiations are allowed in headers.
    if (FD->isTemplateInstantiation())
      return;

    // Member function of a class template and member function of a nested class
    // in a class template are allowed.
    if (const auto *MD = dyn_cast<CXXMethodDecl>(FD)) {
      const auto *DC = MD->getDeclContext();
      while (DC->isRecord()) {
        if (const auto *RD = dyn_cast<CXXRecordDecl>(DC)) {
          if (isa<ClassTemplatePartialSpecializationDecl>(RD))
            return;
          if (RD->getDescribedClassTemplate())
            return;
        }
        DC = DC->getParent();
      }
    }

    // special member functions defaults are prohibited in headers.
    if (FD->isDefaulted()) {
      if (AllowSoleDefaultDtor && dyn_cast<CXXDestructorDecl>(ND)) {
        // the function has already been proven to be marked as defaulted
        // and now we've gotten here as the method is known as a "dtor" so we can
        // assume we're skipping this one.
        //
        // Issue: this allows ALL destructors to be declared in a header.  Currently
        // there is no known method to fix this.
        return;
      }
      diag(FD->getLocation(),
           "special member function %0 defaulted in a header file; "
           "runtimecore prohibits defaults in headers and should be the source "
           "file")
          << FD;
      return;
    }

    if (FD->isLateTemplateParsed()) {
      return;
    }

    if (const auto *MD = dyn_cast<CXXMethodDecl>(FD)) {
      const auto *DC = MD->getDeclContext();
      while (DC->isRecord()) {
        if (const auto *RD = dyn_cast<CXXRecordDecl>(DC)) {
          // If the definition is a lambda type then ignore.
          if (RD->isLambda()) {
            return;
          }

          if (isa<ClassTemplatePartialSpecializationDecl>(RD)) {
            return;
          }

          if (RD->getDescribedClassTemplate()) {
            return;
          }

          if (const auto *TD = RD->getDescribedClassTemplate()) {
            if (TD->isThisDeclarationADefinition()) {
              return;
            }
          }
        }
        DC = DC->getParent();
      }
    }

    // rule of five defaults are prohibited in headers.
    // except for destructors
    if (FD->isDefaulted()) {
      diag(FD->getLocation(),
           "rule of five function %0 defaulted in a header file; "
           "runtimecore prohibits defaults in headers. Please move them to the "
           "source file")
          << FD;
      return;
    }

    if (FD->isInlineSpecified()) {
      diag(FD->getLocation(), "function %0 defined in a header file; "
                              "runtimecore prohibits inline keyword functions")
          << FD;
      return;
    }

    if (FD->isConstexpr()) {
      return;
    }
    // We have a declaration without a compoundStmt.  Move along, as the AST
    // points to us that anytime we have an inline function we also have a
    // compoundStmt.  This may be a false check on some areas but for now
    // it's the best idea we have.
    auto body = FD->getBody();
    if (body && (!dyn_cast<CompoundStmt>(body))) {
      return;
    }

    diag(FD->getLocation(),
         "function %0 defined in a header file; "
         "runtimecore prohibits explicitly defining functions inline")
        << FD;

  } else if (const auto *VD = dyn_cast<VarDecl>(ND)) {
    // Static data members of a class template are allowed.
    if (VD->getDeclContext()->isDependentContext() && VD->isStaticDataMember())
      return;
    if (isTemplateInstantiation(VD->getTemplateSpecializationKind()))
      return;
    // Ignore if the variable is constexpr.
    if (VD->isConstexpr())
      return;
    // Ignore variable definition within function scope.
    if (VD->hasLocalStorage() || VD->isStaticLocal())
      return;

    diag(VD->getLocation(),
         "variable %0 defined in a header file; "
         "runtimecore prohibits variable definitions in header files")
        << VD;
  }
}

} // namespace esri
} // namespace tidy
} // namespace clang
