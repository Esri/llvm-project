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

  auto SizeTType = qualType(hasDeclaration(
    namedDecl(hasName("size_t"))
  ));

  auto ConditionalWithLiterals = conditionalOperator(
    hasTrueExpression(integerLiteral()),
    hasFalseExpression(integerLiteral())
  );

  auto SourceExpr = expr(
    unless(integerLiteral()),                   // ...that isn't a literal
    unless(has(initListExpr(                    // ...or a type initializer (`int32_t{}`)
      has(integerLiteral()))                    // ...that contains a literal
    )),

    unless(ConditionalWithLiterals),            // ...or a ternary operator where both branches are literals
    unless(parenExpr(                           // ...or parenthesis surrounding a
      has(ConditionalWithLiterals)              // ...ternary operator where both branches are literals
    )),

    unless(binaryOperator(                      // ...or a subtraction of two pointers
      hasOperatorName("-"),
      hasLHS(hasType(isAnyPointer())),
      hasRHS(hasType(isAnyPointer()))
    ))
  );

  auto ImplicitIntegralCast = implicitCastExpr( //  Match implicit casts
    hasCastKind(CK_IntegralCast)                //  ...that are integram casts
  ).bind("implicit_cast");

  Finder->addMatcher(
    implicitCastExpr(                           // Match implicit casts where...
      isExpansionInMainFile(),                  // ...the cast is in the file being examined
      ImplicitIntegralCast,                     // ...is an integral cast (as opposed to ValueCategory cast)
      hasImplicitDestinationType(SizeTType),    // ...where the casted to type is size_t
      hasSourceExpression(                      // ...and the source expression
        SourceExpr.bind("se")                   // ...matches the SourceExpr matcher
      )
    ),
    this);

  Finder->addMatcher(
    binaryOperation(
      isAssignmentOperator(),
      hasLHS(hasType(SizeTType)),
      hasRHS(SourceExpr.bind("se"))
    ).bind("assignment"),
    this);

  Finder->addMatcher(
    varDecl(                                    // Match any variable declarations
      hasType(SizeTType),                       // ...of type size_t
      hasInitializer(expr(                      // ...where the init expression
        SourceExpr.bind("se"),                  // ...matches the SourceExpr matcher
        unless(anyOf(                           // ...but the expression
          ImplicitIntegralCast,                 // ...isn't an integral cast itself
          hasDescendant(ImplicitIntegralCast)   // ...or comes from an integral cast
        ))
      ))
    ).bind("var"),
    this);

  Finder->addMatcher(
    callExpr(                                   // Match function calls
      forEachArgumentWithParam(                 // ...where any of the arguments
        declRefExpr(SourceExpr.bind("se")),     // ...matches the SourceExpr matcher
        parmVarDecl(hasType(SizeTType))         // ...and the parameter type is size_t
      )
    ),
    this);
}

void ImplicitCastToSizetCheck::check(const MatchFinder::MatchResult &Result)
{
  if (const auto *MatchedExpr = Result.Nodes.getNodeAs<ImplicitCastExpr>("implicit_cast"))
  {
    if (MatchedExpr->isPartOfExplicitCast())
      return;
  }

  const auto *SourceExpr = Result.Nodes.getNodeAs<Expr>("se");
  assert(SourceExpr && "Needs to have a source Expr");

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

  diag(SourceExpr->getBeginLoc(), "implicit cast to size_t may be a narrowing cast")
      << SourceExpr->getSourceRange();
}

} // namespace esri
} // namespace tidy
} // namespace clang
