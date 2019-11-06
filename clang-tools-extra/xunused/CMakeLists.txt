add_clang_executable(xunused
  main.cpp
  )

target_link_libraries(xunused
  PRIVATE
  clangIndex
  clangTooling
  clangFrontend
  clangDriver
  clangSerialization
  clangParse
  clangSema
  clangAnalysis
  clangASTMatchers
  clangAST
  clangEdit
  clangLex
  clangBasic  
)

set_target_properties(xunused PROPERTIES VERSION ${CLANG_EXECUTABLE_VERSION})

add_dependencies(xunused clang-headers)

install(TARGETS xunused
  RUNTIME DESTINATION bin COMPONENT xunused)

add_llvm_install_targets(install-xunused DEPENDS xunused COMPONENT xunused)
