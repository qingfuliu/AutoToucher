cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy.
set(CMAKE_CXX_STANDARD 17) # Compile as C++17.
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Require C++17 support.
include(GNUInstallDirs)


project(AutoToucher
        VERSION 1.0
        DESCRIPTION "An automatic clicker based on small target recognition"
        LANGUAGES C CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to `Debug` as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

if (EXISTS "${PATH_TO_CMAKELISTS_TXT}")
    message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
    Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif ()

if (CMAKE_CXX_COMPILER_ID)
    if (CMAKE_CXX_COMPILER_VERSION)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
            message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}, which is version ${CMAKE_CXX_COMPILER_VERSION}. Please ensure that its version is greater than 14")
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.4.0)
            message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID}, which is version ${CMAKE_CXX_COMPILER_VERSION}. Please ensure that its version is greater than 11.4.0")
        endif ()
        message(STATUS "You're using ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    else ()
        message(WARNING "You're using ${CMAKE_CXX_COMPILER_ID} , but the version number has not been detected")
        message(WARNING "If you are using gcc, please make sure its version number is greater than 11.4.0.
                         If you are using clang, please make sure its version number is greater than 14")
    endif ()
endif ()

set(AUTOTOUCHER_BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build_support")
string(CONCAT AUTOTOUCHER_FORMAT_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)


# #####################################################################################################################
# DEPENDENCIES
# #####################################################################################################################
# clang-format
# attempt to find the binary if user did not specify
find_program(CLANG_FORMAT_BIN
        NAMES clang-format clang-format-17
        HINTS ${AUTOTOUCHER_CLANG_SEARCH_PATH})

if ("${CLANG_FORMAT_BIN}" STREQUAL "CLANG_FORMAT_BIN-NOTFOUND")
    message(WARNING "AUTOTOUCHER/main couldn't find clang-format.")
else ()
    message(STATUS "AUTOTOUCHER/main found clang-format at ${CLANG_FORMAT_BIN}")
endif ()

# attempt to find the binary if user did not specify
find_program(CLANG_TIDY_BIN
        NAMES clang-tidy clang-tidy-17
        HINTS ${AUTOTOUCHER_CLANG_SEARCH_PATH})

if ("${CLANG_TIDY_BIN}" STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
    message(WARNING "AUTOTOUCHER/main couldn't find clang-tidy.")
else ()
    # Output compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
    message(STATUS "AUTOTOUCHER/main found clang-tidy at ${CLANG_TIDY_BIN}")
endif ()

find_program(CLANG_APPLY_REPLACEMENTS_BIN
        NAMES clang-apply-replacements clang-apply-replacements-17
        HINTS ${AUTOTOUCHER_CLANG_SEARCH_PATH})

if ("${CLANG_APPLY_REPLACEMENTS_BIN}" STREQUAL "CLANG_APPLY_REPLACEMENTS_BIN-NOTFOUND")
    message(WARNING "AUTOTOUCHER/main couldn't find clang-apply-replacements.")
else ()
    # Output compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
    message(STATUS "AUTOTOUCHER/main found clang-apply-replacements at ${CLANG_APPLY_REPLACEMENTS_BIN}")
endif ()

# cpplint
find_program(CPPLINT_BIN
        NAMES cpplint cpplint.py
        HINTS "${AUTOTOUCHER_BUILD_SUPPORT_DIR}")

if ("${CPPLINT_BIN}" STREQUAL "CPPLINT_BIN-NOTFOUND")
    message(WARNING "AUTOTOUCHER/main couldn't find cpplint.")
else ()
    message(STATUS "AUTOTOUCHER/main found cpplint at ${CPPLINT_BIN}")
endif ()

# Runs clang format and updates files in place.
add_custom_target(format ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/clang_format_exclusions.txt
        --source_dirs
        ${AUTOTOUCHER_FORMAT_DIRS}
        --fix
        --quiet
)

# Runs clang format and exits with a non-zero exit code if any files need to be reformatted
add_custom_target(check-format ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/clang_format_exclusions.txt
        --source_dirs
        ${AUTOTOUCHER_FORMAT_DIRS}
        --quiet
)

# #########################################
# "make check-lint"
# #########################################
file(GLOB_RECURSE AUTOTOUCHER_LINT_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
# Balancing act: cpplint.py takes a non-trivial time to launch,
# so process 12 files per invocation, while still ensuring parallelism
add_custom_target(check-lint echo '${AUTOTOUCHER_LINT_FILES}' | xargs -n12 -P8
        ${CPPLINT_BIN}
        --verbose=2 --quiet
        --linelength=120
        --filter=-legal/copyright,-build/header_guard,-runtime/references
)

set(CHECK_CLANG_TIDY_FILES
        "main.cpp"
        "src/include/configurator/auto_config.h"
)
# ##########################################################
# "make check-clang-tidy" target
# ##########################################################
# runs clang-tidy and exits with a non-zero exit code if any errors are found.
# note that clang-tidy automatically looks for a .clang-tidy file in parent directories
add_custom_target(check-clang-tidy
        ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        ${CHECK_CLANG_TIDY_FILES}
)

message("${AUTOTOUCHER_BUILD_SUPPORT_DIR}/run_clang_tidy.py -clang-tidy-binary ${CLANG_TIDY_BIN} -p ${CMAKE_BINARY_DIR} ${CHECK_CLANG_TIDY_FILES}")

add_custom_target(fix-clang-tidy
        ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -clang-apply-replacements-binary ${CLANG_APPLY_REPLACEMENTS_BIN} # using our clang-apply-replacements binary
        -fix # apply suggested changes generated by clang-tidy
)

add_custom_target(check-clang-tidy-diff
        ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -only-diff # only check diff files to master
)
add_custom_target(fix-clang-tidy-diff
        ${AUTOTOUCHER_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN} # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR} # using cmake's generated compile commands
        -clang-apply-replacements-binary ${CLANG_APPLY_REPLACEMENTS_BIN} # using our clang-apply-replacements binary
        -fix # apply suggested changes generated by clang-tidy
        -only-diff # only check diff files to master
)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

# Output directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("Build mode: ${CMAKE_BUILD_TYPE}")
add_executable(main main.cpp src/include/configurator/auto_config.h)
target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/src/include)

if (NOT DEFINED ENABLE_TEST)
    set(ENABLE_TEST "True")
endif ()

if (ENABLE_TEST STREQUAL "True")
    enable_testing()
endif ()
add_subdirectory(./third_part)
add_subdirectory(./src)
add_subdirectory(./test)
add_dependencies(check-clang-tidy main) # needs gtest headers, compile_commands.json
