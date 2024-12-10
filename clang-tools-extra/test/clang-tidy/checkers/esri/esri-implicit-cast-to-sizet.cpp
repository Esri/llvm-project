// RUN: %check_clang_tidy %s esri-implicit-cast-to-sizet %t

#include <stddef.h>
#include <stdint.h>

// Verified cases where a warning is raised

// Several PRs had this specific problem:
//   * #29277
//   * #29456
//   * #29516
void warning_on_uint64()
{
  uint64_t input{1};
  size_t var = input;
  // CHECK-MESSAGES: :[[@LINE-1]]:16: warning: implicit cast to size_t may be a narrowing cast
}

// PR #29503
void warning_on_longlong()
{
  long long input{1};
  size_t var = input;
  // CHECK-MESSAGES: :[[@LINE-1]]:16: warning: implicit cast to size_t may be a narrowing cast
}

/*
// TODO
// PR #29516 this different but related warning:
// warning C4293: '>>': shift count negative or too big, undefined behavior
void DIFFERENT_WARNING()
{
  size_t input{1};
  uint64_t var = (input >> 32);
  // TODO-CHECK-MESSAGES: :[[@LINE-1]]:16: warning: implicit cast to size_t may be a narrowing cast
}
*/

// NO PR, just additional tests
void warning_on_adding_variants()
{
  // NEEDED?
  uint64_t base = 1;
  uint64_t offset = 2;
  size_t var = base + offset;
  // CHECK-MESSAGES: :[[@LINE-1]]:16: warning: implicit cast to size_t may be a narrowing cast
}

void d(size_t);

void warning_when_implicit_passing_value_as_func_argument()
{
  uint64_t v{3};
  d(v); // Follows the same rules as a function argument as init expr
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: implicit cast to size_t may be a narrowing cast
}

void warning_from_subtraction_of_int64()
{
  int64_t input1{2};
  int64_t input2{1};
  size_t val = input1 - input2;
  // CHECK-MESSAGES: :[[@LINE-1]]:16: warning: implicit cast to size_t may be a narrowing cast
}

// Verified cases where no warning is raised

void no_warning_for_literals()
{
  size_t i = int{1};
}

void no_warning_for_int32()
{
  int32_t signed_int{1};
  size_t val = signed_int;
}

void no_warning_for_literals_if_no_truncation()
{
  size_t offset_signed = int64_t{1};
  size_t offset_unsigned = uint64_t{1000};
  size_t offset_longlong = (long long)10000;
}

void no_warning_from_subtraction_of_int32()
{
  int32_t input1{2};
  int32_t input2{1};
  size_t val = input1 - input2;
}

void no_warning_from_subtraction_of_pointers()
{
  const char* text_begin = "Hello";
  const char* text_end = "World";
  size_t num_remaining = text_end - text_begin;
}

// Other things that could trigger a false positive

void no_warning_when_comparing_to_int32_literal()
{
  size_t v{100};
  bool b = (v == 0);
}

// When invoking a static member function that returns a value that's then
// casted to size_t, there's an implicit nested cast for some reason:
//
// >> `-CXXStaticCastExpr <col:14, col:42> 'size_t':'unsigned long' static_cast<size_t> <NoOp>
// >>   `-ImplicitCastExpr <col:34, col:41> 'size_t':'unsigned long' <IntegralCast> part_of_explicit_cast
//       `-CallExpr <col:34, col:41> 'int'
//         `-ImplicitCastExpr <col:34, col:37> 'int (*)()' <FunctionToPointerDecay>
//           `-DeclRefExpr <col:34, col:37> 'int ()' lvalue CXXMethod 0xdcca750 'too' 'int ()'
//             `-NestedNameSpecifier TypeSpec 'D'
//
// The check needs to check the isPartOfExplicitCast() bit

class D
{
public:
  static int too();
};

void no_warning_when_static_method_invoked_in_explicit_cast()
{
  auto val = static_cast<size_t>(D::too());
}

// Converting a branch from an ternary expression from an integer literal to a
// size_t should follow the same logic as a regular literal if both branches are
// literals
void no_warning_when_casting_literals_from_ternary_expr()
{
  size_t i{0};
  i += false ? 0 : 1;
  i += (true ? 0 : 1);
}

// Should follow the same literal rules even if when passing as an argument
// instead of initializing a variable
void no_warning_when_passing_casted_argument_with_literal()
{
  d(uint64_t{3});
}
