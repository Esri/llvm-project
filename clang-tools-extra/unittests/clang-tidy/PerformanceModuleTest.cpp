#include "ClangTidyTest.h"
#include "performance/UnnecessaryValueParamCheck.h"
#include "ClangTidyDiagnosticConsumer.h"
#include "gtest/gtest.h"

namespace clang {
namespace tidy {
namespace test {

using performance::UnnecessaryValueParamCheck;

TEST(UnnecessaryValueParamCheckTest, Basic) {
  const std::string TestCode = R"(
    namespace std {
      template<typename>
      struct remove_reference;

      template<typename _Tp>
      struct remove_reference {
        typedef _Tp type;
      };  

      template<typename _Tp>
      struct remove_reference<_Tp &> {
        typedef _Tp type;
      };

      template<typename _Tp>
      struct remove_reference<_Tp &&> {
        typedef _Tp type;
      };

      template<typename _Ty>
      constexpr typename remove_reference<_Ty>::type&& move(_Ty &&arg) noexcept {
          return static_cast<typename remove_reference<_Ty>::type&&>(arg);        
      }
    } // namespace std

    class ExpensiveToCopy {
      int member;

      public:
      ExpensiveToCopy& operator=(const ExpensiveToCopy &arg) {
          this->member = arg.member;
          return *this;
      }
    };

    void MoveExpensiveToCopyType(ExpensiveToCopy arg) {
      auto arg2 = std::move(arg);
    }
  )";  
   EXPECT_NO_CHANGES(UnnecessaryValueParamCheck, TestCode);
  }
}
}
}
