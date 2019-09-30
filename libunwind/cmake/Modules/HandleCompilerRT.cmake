function(find_compiler_rt_library name dest)
  if (NOT DEFINED LIBUNWIND_COMPILE_FLAGS)
    message(FATAL_ERROR "LIBUNWIND_COMPILE_FLAGS must be defined when using this function")
  endif()
  set(dest "" PARENT_SCOPE)
  set(CLANG_COMMAND ${CMAKE_CXX_COMPILER} ${LIBUNWIND_COMPILE_FLAGS}
      "--rtlib=compiler-rt" "--print-libgcc-file-name")
  if (CMAKE_CXX_COMPILER_ID MATCHES Clang AND CMAKE_CXX_COMPILER_TARGET)
    list(APPEND CLANG_COMMAND "--target=${CMAKE_CXX_COMPILER_TARGET}")
  endif()
  get_property(LIBUNWIND_CXX_FLAGS CACHE CMAKE_CXX_FLAGS PROPERTY VALUE)
  string(REPLACE " " ";" LIBUNWIND_CXX_FLAGS "${LIBUNWIND_CXX_FLAGS}")
  list(APPEND CLANG_COMMAND ${LIBUNWIND_CXX_FLAGS})
  execute_process(
      COMMAND ${CLANG_COMMAND}
      RESULT_VARIABLE HAD_ERROR
      OUTPUT_VARIABLE LIBRARY_FILE
  )
  string(STRIP "${LIBRARY_FILE}" LIBRARY_FILE)
  file(TO_CMAKE_PATH "${LIBRARY_FILE}" LIBRARY_FILE)
  string(REPLACE "builtins" "${name}" LIBRARY_FILE "${LIBRARY_FILE}")
  if (NOT HAD_ERROR AND EXISTS "${LIBRARY_FILE}")
    message(STATUS "Found compiler-rt library: ${LIBRARY_FILE}")
    set(${dest} "${LIBRARY_FILE}" PARENT_SCOPE)
  else()
    message(STATUS "Failed to find compiler-rt library")
  endif()
endfunction()

function(find_compiler_rt_dir dest)
  if (NOT DEFINED LIBUNWIND_COMPILE_FLAGS)
    message(FATAL_ERROR "LIBUNWIND_COMPILE_FLAGS must be defined when using this function")
  endif()
  set(dest "" PARENT_SCOPE)
  if (APPLE)
    set(CLANG_COMMAND ${CMAKE_CXX_COMPILER} ${LIBUNWIND_COMPILE_FLAGS}
        "-print-file-name=lib")
    execute_process(
        COMMAND ${CLANG_COMMAND}
        RESULT_VARIABLE HAD_ERROR
        OUTPUT_VARIABLE LIBRARY_DIR
    )
    string(STRIP "${LIBRARY_DIR}" LIBRARY_DIR)
    file(TO_CMAKE_PATH "${LIBRARY_DIR}" LIBRARY_DIR)
    set(LIBRARY_DIR "${LIBRARY_DIR}/darwin")
  else()
    set(CLANG_COMMAND ${CMAKE_CXX_COMPILER} ${LIBUNWIND_COMPILE_FLAGS}
        "--rtlib=compiler-rt" "--print-libgcc-file-name")
    execute_process(
        COMMAND ${CLANG_COMMAND}
        RESULT_VARIABLE HAD_ERROR
        OUTPUT_VARIABLE LIBRARY_FILE
    )
    string(STRIP "${LIBRARY_FILE}" LIBRARY_FILE)
    file(TO_CMAKE_PATH "${LIBRARY_FILE}" LIBRARY_FILE)
    get_filename_component(LIBRARY_DIR "${LIBRARY_FILE}" DIRECTORY)
  endif()
  if (NOT HAD_ERROR AND EXISTS "${LIBRARY_DIR}")
    message(STATUS "Found compiler-rt directory: ${LIBRARY_DIR}")
    set(${dest} "${LIBRARY_DIR}" PARENT_SCOPE)
  else()
    message(STATUS "Failed to find compiler-rt directory")
  endif()
endfunction()

# Mangle the name of a compiler flag into a valid CMake identifier.
# Ex: --std=c++11 -> STD_EQ_CXX11
macro(mangle_name str output)
  string(STRIP "${str}" strippedStr)
  string(REGEX REPLACE "^/" "" strippedStr "${strippedStr}")
  string(REGEX REPLACE "^-+" "" strippedStr "${strippedStr}")
  string(REGEX REPLACE "-+$" "" strippedStr "${strippedStr}")
  string(REPLACE "-" "_" strippedStr "${strippedStr}")
  string(REPLACE "=" "_EQ_" strippedStr "${strippedStr}")
  string(REPLACE "+" "X" strippedStr "${strippedStr}")
  string(TOUPPER "${strippedStr}" ${output})
endmacro()

# If the specified 'condition' is true then append the specified list of flags to DEST
macro(append_flags_if condition DEST)
  if (${condition})
    list(APPEND ${DEST} ${ARGN})
  endif()
endmacro()

# Add each flag in the list specified by DEST if that flag is supported by the current compiler.
macro(append_flags_if_supported DEST)
  foreach(flag ${ARGN})
    mangle_name("${flag}" flagname)
    check_cxx_compiler_flag("${flag}" "LIBUNWIND_SUPPORTS_${flagname}_FLAG")
    append_flags_if(LIBUNWIND_SUPPORTS_${flagname}_FLAG ${DEST} ${flag})
  endforeach()
endmacro()