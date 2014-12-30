project(libsystemtray)
cmake_minimum_required(VERSION 3.0)


include(GNUInstallDirs)

find_package(PkgConfig)
pkg_check_modules(DBUS dbus-1 REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++11") 

add_library(systemtray SHARED src/kdetrayicon.cpp src/slimdbus.cpp)
include_directories(${DBUS_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(systemtray ${CMAKE_DL_LIBS})

add_executable(iconbyname tests/iconbyname.cpp)
target_link_libraries(iconbyname systemtray)

add_executable(iconbyname_c tests/iconbyname.c)
target_link_libraries(iconbyname_c systemtray)

install(DIRECTORY include/systemtray DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS systemtray LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})