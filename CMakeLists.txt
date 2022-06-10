cmake_minimum_required(VERSION 3.4...3.22)

project(picologging)

find_package(PythonExtensions REQUIRED)

add_library(_picologging MODULE picologging/_picologging.cxx picologging/logrecord.cxx picologging/formatstyle.cxx picologging/formatter.cxx)
set_target_properties(_picologging PROPERTIES CXX_STANDARD 17)
python_extension_module(_picologging)
install(TARGETS _picologging LIBRARY DESTINATION picologging)