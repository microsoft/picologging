cmake_minimum_required(VERSION 3.4...3.22)

project(picologging)

find_package(PythonExtensions REQUIRED)

add_library(_picologging MODULE src/picologging/_picologging.cxx src/picologging/logrecord.cxx src/picologging/formatstyle.cxx src/picologging/formatter.cxx src/picologging/logger.cxx src/picologging/handler.cxx src/picologging/filterer.cxx  src/picologging/streamhandler.cxx)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
    set_target_properties(_picologging PROPERTIES CXX_STANDARD 17)
endif (MSVC)

python_extension_module(_picologging)
install(TARGETS _picologging LIBRARY DESTINATION src/picologging)