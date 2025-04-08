# CMake generated Testfile for 
# Source directory: C:/Users/HydraPony/1337 h4x0r/TMNT AI
# Build directory: C:/Users/HydraPony/1337 h4x0r/TMNT AI
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(RenderTests "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/Debug/RenderTests.exe")
  set_tests_properties(RenderTests PROPERTIES  WORKING_DIRECTORY "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/Debug" _BACKTRACE_TRIPLES "C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;123;add_test;C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(RenderTests "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/Release/RenderTests.exe")
  set_tests_properties(RenderTests PROPERTIES  WORKING_DIRECTORY "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/Release" _BACKTRACE_TRIPLES "C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;123;add_test;C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(RenderTests "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/MinSizeRel/RenderTests.exe")
  set_tests_properties(RenderTests PROPERTIES  WORKING_DIRECTORY "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/MinSizeRel" _BACKTRACE_TRIPLES "C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;123;add_test;C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(RenderTests "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/RelWithDebInfo/RenderTests.exe")
  set_tests_properties(RenderTests PROPERTIES  WORKING_DIRECTORY "C:/Users/HydraPony/1337 h4x0r/TMNT AI/bin/RelWithDebInfo" _BACKTRACE_TRIPLES "C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;123;add_test;C:/Users/HydraPony/1337 h4x0r/TMNT AI/CMakeLists.txt;0;")
else()
  add_test(RenderTests NOT_AVAILABLE)
endif()
subdirs("deps/googletest")
