set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyEsriModule
  EsriTidyModule.cpp
  NoImplementationInHeadersCheck.cpp

  LINK_LIBS
  clangTidy
  clangTidyUtils
  )

clang_target_link_libraries(clangTidyEsriModule
  PRIVATE
  clangAnalysis
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  )
