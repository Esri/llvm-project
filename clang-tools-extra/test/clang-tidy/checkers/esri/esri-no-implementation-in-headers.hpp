// RUN: %check_clang_tidy %s esri-no-implementation-in-headers %t

inline int f() { // CHECK-MESSAGES: :[[@LINE]]:12: warning: function 'f' defined in a header file;
  return 1;
}

class CA {               // OK: class definition
  int f1() { return 2; } // CHECK-MESSAGES: :[[@LINE]]:7: warning: function 'f1' defined in a header file;
  void f2();             // OK: out of line definition
  template <typename T>
  T f3() {
    T a = 1;
    return a;
  }
  template <typename T>
  struct CAA {
    struct CAB {
      void f4();
    };
  };
  void f5() { return; }; // CHECK-MESSAGES: :[[@LINE]]:8: warning: function 'f5' defined in a header file;
};

void CA::f2() {} // CHECK-MESSAGES: :[[@LINE]]:10: warning: function 'f2' defined in a header file;

class DA {
  DA() = default;                      // CHECK-MESSAGES: :[[@LINE]]:3: warning: special member function 'DA' defaulted in a header file;
  DA(const DA &) = default;            // CHECK-MESSAGES: :[[@LINE]]:3: warning: special member function 'DA' defaulted in a header file;
  virtual ~DA() = default;             // OK: the function being marked default is a destructor
  DA &operator=(const DA &) = default; // CHECK-MESSAGES: :[[@LINE]]:7: warning: special member function 'operator=' defaulted in a header file;
  DA &operator=(DA &&) = default;      // CHECK-MESSAGES: :[[@LINE]]:7: warning: special member function 'operator=' defaulted in a header file;
};

class DB {
  DB() = delete;                      // OK: the function being marked delete is not callable.
  DB(const DB &) = delete;            // OK: the function being marked delete is not callable.
  virtual ~DB() = delete;             // OK: the function being marked delete is not callable.
  DB &operator=(const DB &) = delete; // OK: the function being marked delete is not callable.
  DB &operator=(DB &&) = delete;      // OK: the function being marked delete is not callable.
};

class DC {
  virtual ~DC() = default;             // OK: the function being marked default is a destructor
};

template <>
int CA::f3() { // CHECK-MESSAGES: :[[@LINE]]:9: warning: explicit template specialization 'f3<int>' defined in a header file;
  int a = 1;
  return a;
}

template <typename T>
void CA::CAA<T>::CAB::f4() {
  // OK: member function definition of a nested template class in a class.
}

template <typename T>
struct CB {
  void f1();
  struct CCA {
    void f2(T a);
  };
  struct CCB;   // OK: forward declaration.
  static int a; // OK: class static data member declaration.
};

template <typename T>
void CB<T>::f1() { // OK: Member function definition of a class template.
}

template <typename T>
void CB<T>::CCA::f2(T a) {
  // OK: member function definition of a nested class in a class template.
}

template <typename T>
struct CB<T>::CCB {
  void f3();
};

template <typename T>
void CB<T>::CCB::f3() {
  // OK: member function definition of a nested class in a class template.
}

template <typename T>
int CB<T>::a = 2; // OK: static data member definition of a class template.

template <typename T>
T tf() { // OK: template function definition.
  T a;
  return a;
}

namespace NA {
int f() { return 1; } // CHECK-MESSAGES: :[[@LINE]]:5: warning: function 'f' defined in a header file;
} // namespace NA

template <typename T>
T f3() {
  T a = 1;
  return a;
}

template <>
int f3() { // CHECK-MESSAGES: :[[@LINE]]:5: warning: explicit template specialization 'f3<int>' defined in a header file;
  int a = 1;
  return a;
}

template <>
inline double f3() { // CHECK-MESSAGES: :[[@LINE]]:15: warning: explicit template specialization 'f3<double>' defined in a header file;
  return 0;
}
extern template double f3(); // extern the inline function in order to force the use of it

int f5();                     // OK: function declaration.
inline int f6() { return 1; } // CHECK-MESSAGES: :[[@LINE]]:12: warning: function 'f6' defined in a header file;
namespace {
int f7() { return 1; } // CHECK-MESSAGES: :[[@LINE]]:5: warning: anonymously namespaced 'f7' defined in a header file;
} // namespace

int f8() = delete; // OK: the function being marked delete is not callable.

int a = 1; // CHECK-MESSAGES: :[[@LINE]]:5: warning: variable 'a' defined in a header file;
CA a1;     // CHECK-MESSAGES: :[[@LINE]]:4: warning: variable 'a1' defined in a header file;

namespace NB {
int b = 1;       // CHECK-MESSAGES: :[[@LINE]]:5: warning: variable 'b' defined in a header file;
const int c = 1; // OK: internal linkage variable definition.
} // namespace NB

class CC {
  static int d; // OK: class static data member declaration.
};

int CC::d = 1; // CHECK-MESSAGES: :[[@LINE]]:9: warning: variable 'd' defined in a header file;

const char *ca = "foo"; // CHECK-MESSAGES: :[[@LINE]]:13: warning: variable 'ca' defined in a header file;

namespace {
int e = 2; // CHECK-MESSAGES: :[[@LINE]]:5: warning: anonymously namespaced 'e' defined in a header file;
} // namespace

// Even if a definition is constexpr, anonymous namespaces are not allowed.
namespace {
constexpr int invalid_constexpr_var = 2; // CHECK-MESSAGES: :[[@LINE]]:15: warning: anonymously namespaced 'invalid_constexpr_var' defined in a header file;
} // namespace

const char *const g = "foo"; // OK: internal linkage variable definition.
static int h = 1;            // OK: internal linkage variable definition.
const int i = 1;             // OK: internal linkage variable definition.
extern int j;                // OK: internal linkage variable definition.

template <typename T, typename U>
struct CD {
  int f();
};

template <typename T>
struct CD<T, int> {
  int f();
};

template <>
struct CD<int, int> {
  int f();
};

int CD<int, int>::f() { // CHECK-MESSAGES: :[[@LINE]]:19: warning: function 'f' defined in a header file;
  return 0;
}

template <typename T>
int CD<T, int>::f() { // OK: partial template specialization.
  return 0;
}

template <class T>
class EA
{
  bool var{false};

public:
  EA() = default; // OK: templated function
  ~EA() = default; // OK: templated function
};

class StaticConstExpr {
  static constexpr int v() // OK: the function is constexpr
  {
    return 1;
  }
  inline static constexpr int v2() // CHECK-MESSAGES: :[[@LINE]]:31: warning: function 'v2' defined in a header file;
  {
    return 2;
  }
};

constexpr int foo = 42; // OK: constexpr variable defined in header.

// Helper struct for defining a constexpr variable in a template.
template<typename T>
struct AllowedVariable {
  static constexpr bool value = true;
};

template<typename T>
constexpr bool allowed_variable_value = AllowedVariable<T>::value; // OK: constexpr variable defined in template.

template<typename Fn>
int DoubleValue(int n, Fn&& fn)
{
  return fn(n);
}

template<int N>
int callDoubleValue()
{
  // OK: lambda is an implicitly inline operator() with definition, allowed in headers.
  return DoubleValue(N, [](const int v){ return v*2;});
}
