cmake_minimum_required(VERSION 3.15...3.30)

option(COVERAGE "Enable coverage reporting" OFF)
option(CACHE_FILEPATH "Enable cache filepath" ON)
project(${SKBUILD_PROJECT_NAME})

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

python_add_library(_picologging MODULE WITH_SOABI src/picologging/_picologging.cxx src/picologging/logrecord.cxx src/picologging/formatstyle.cxx src/picologging/formatter.cxx src/picologging/logger.cxx src/picologging/handler.cxx src/picologging/filterer.cxx  src/picologging/streamhandler.cxx src/picologging/filepathcache.cxx)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
    set_target_properties(_picologging PROPERTIES CXX_STANDARD 17)
endif (MSVC)

if (CACHE_FILEPATH)
    add_definitions(-DPICOLOGGING_CACHE_FILEPATH)
endif (CACHE_FILEPATH)

if(COVERAGE)
    message(STATUS "Enabling coverage")
    # Add required flags (GCC & LLVM/Clang)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(_picologging PRIVATE
                -O0        # no optimization
                -g         # generate debug info
                --coverage # sets all required flags
                -fprofile-abs-path
                )
        target_link_options(_picologging PRIVATE --coverage)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(_picologging PRIVATE
                -O0        # no optimization
                -g         # generate debug info
                -fprofile-instr-generate -fcoverage-mapping # coverage flags
                )
        target_link_options(_picologging PRIVATE --coverage)
    endif()
endif(COVERAGE)

if(FUZZING)
    target_compile_options(_picologging PRIVATE -fsanitize=address,fuzzer)
    target_link_options(_picologging PRIVATE -fsanitize=address,fuzzer)
endif(FUZZING)

install(TARGETS _picologging LIBRARY DESTINATION ${SKBUILD_PROJECT_NAME})
