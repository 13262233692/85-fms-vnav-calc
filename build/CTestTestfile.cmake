# CMake generated Testfile for 
# Source directory: D:/SOLO-10/85-fms-vnav-calc
# Build directory: D:/SOLO-10/85-fms-vnav-calc/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[BADAValidationTest]=] "D:/SOLO-10/85-fms-vnav-calc/build/bin/Debug/BADAValidationTest.exe")
  set_tests_properties([=[BADAValidationTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;143;add_test;D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[BADAValidationTest]=] "D:/SOLO-10/85-fms-vnav-calc/build/bin/Release/BADAValidationTest.exe")
  set_tests_properties([=[BADAValidationTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;143;add_test;D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[BADAValidationTest]=] "D:/SOLO-10/85-fms-vnav-calc/build/bin/MinSizeRel/BADAValidationTest.exe")
  set_tests_properties([=[BADAValidationTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;143;add_test;D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[BADAValidationTest]=] "D:/SOLO-10/85-fms-vnav-calc/build/bin/RelWithDebInfo/BADAValidationTest.exe")
  set_tests_properties([=[BADAValidationTest]=] PROPERTIES  _BACKTRACE_TRIPLES "D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;143;add_test;D:/SOLO-10/85-fms-vnav-calc/CMakeLists.txt;0;")
else()
  add_test([=[BADAValidationTest]=] NOT_AVAILABLE)
endif()
