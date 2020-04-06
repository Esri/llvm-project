#include "ClangTidyTest.h"
#include "performance/UnnecessaryValueParamCheck.h"
#include "ClangTidyDiagnosticConsumer.h"
#include "gtest/gtest.h"

namespace clang {
namespace tidy {
namespace test {

using performance::UnnecessaryValueParamCheck;

TEST(UnnecessaryValueParamCheckTest, Basic) {
  EXPECT_NO_CHANGES(UnnecessaryValueParamCheck, "#include <string>\n"
                                                "#include <utility>\n"
                                                "void setValue(std::string val) {"
                                                "  std::string field = \"Hello World!\";"
                                                "  field = std::move(val);"
                                                "}");
}

}
}
}
