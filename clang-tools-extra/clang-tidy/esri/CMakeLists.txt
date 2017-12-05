set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyEsriModule
  EsriTidyModule.cpp
  NoImplementationInHeadersCheck.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTidy
  clangTidyUtils
  clangTooling
  )
