cmake_minimum_required(VERSION 3.1)

project(picoKVM2VNC VERSION 1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(VNCS REQUIRED libvncserver)
pkg_check_modules(V4L REQUIRED libv4l2)
pkg_check_modules(V4LC REQUIRED libv4lconvert)


add_executable(picoKVM2VNC main.cpp keymapper.cpp)
target_include_directories(picoKVM2VNC PUBLIC ${VNCS_INCLUDE_DIRS} ${V4L_INCLUDE_DIRS} ${V4LC_INCLUDE_DIRS})
target_compile_options(picoKVM2VNC PUBLIC ${VNCS_CFLAGS_OTHER} ${V4L_CFLAGS_OTHER} ${V4LC_CFLAGS_OTHER})
target_link_libraries(picoKVM2VNC ${VNCS_LIBRARIES} ${V4L_LIBRARIES} ${V4LC_LIBRARIES})
