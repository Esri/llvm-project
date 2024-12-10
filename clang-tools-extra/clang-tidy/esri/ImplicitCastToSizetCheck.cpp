//===--- ImplicitCastToSizetCheck.cpp - clang-tidy ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ImplicitCastToSizetCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace esri {

void ImplicitCastToSizetCheck::registerMatchers(MatchFinder *Finder) {

  auto ConditionalWithLiterals = conditionalOperator(
    hasTrueExpression(integerLiteral()),
    hasFalseExpression(integerLiteral())
  );

  Finder->addMatcher(
    implicitCastExpr(                     // Match implicit casts where...
      isExpansionInMainFile(),            // ...the cast is in the file being examined
      hasCastKind(CK_IntegralCast),       // ...is an integral cast (as opposed to ValueCategory cast)
      hasType(qualType(hasDeclaration(    // ...the type being casted to
        namedDecl(hasName("size_t"))      // ...is size_t
      ))),                                // ...and
      hasSourceExpression(expr(           // ...the source expression

        unless(integerLiteral()),         // ...that isn't a literal
        unless(has(initListExpr(          // ...or a type initializer (`int32_t{}`)
          has(integerLiteral()))          // ...that contains a literal
       )),

        unless(ConditionalWithLiterals),  // ...or a ternary operator where both branches are literals
        unless(parenExpr(                 // ...or parenthesis surrounding a
          has(ConditionalWithLiterals)    // ...ternary operator where both branches are literals
        )),

        unless(binaryOperator(            // ...or a subtraction of two pointers
          hasOperatorName("-"),
          hasLHS(hasType(isAnyPointer())),
          hasRHS(hasType(isAnyPointer()))
        ))
      ).bind("se"))
    ).bind("implicit_cast"),
    this);
}

void ImplicitCastToSizetCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MatchedExpr = Result.Nodes.getNodeAs<ImplicitCastExpr>("implicit_cast");
  if (MatchedExpr->isPartOfExplicitCast())
    return;

  const auto *SourceExpr = Result.Nodes.getNodeAs<Expr>("se");
  if (Result.Context->getTypeSize(SourceExpr->getType()) <= 32)
    return;

  // See if the source expression can be constant evaluated (such as literals)
  if (const auto ConstExprResult =
       SourceExpr->getIntegerConstantExpr(*Result.Context))
  {
    const auto ExprResultWidth = ConstExprResult->getBitWidth();

    // If the expression was a 32-bit integer, just make sure the result is
    // not a negative value
    if (ExprResultWidth == 32 && !ConstExprResult->isNegative())
      return;

    // Make sure the constant value was within the valid range for a 32-bit
    // unsigned integer (size_t on windows_x86)
    auto MaxValue = llvm::APSInt::getMaxValue(32, true);
    MaxValue = MaxValue.extOrTrunc(ExprResultWidth);
    MaxValue.setIsSigned(ConstExprResult->isSigned());

    auto MinValue = llvm::APSInt::getMinValue(32, true);
    MinValue = MinValue.extOrTrunc(ExprResultWidth);
    MinValue.setIsSigned(ConstExprResult->isSigned());

    if (*ConstExprResult <= MaxValue && *ConstExprResult >= MinValue)
      return;
  }

  diag(MatchedExpr->getBeginLoc(), "implicit cast to size_t may be a narrowing cast")
      << SourceExpr->getSourceRange();
}

} // namespace esri
} // namespace tidy
} // namespace clang
