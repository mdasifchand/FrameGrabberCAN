cmake_minimum_required(VERSION 3.5.1)
project(basler_test)
set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl, -E")


#creating the library target
find_package(OpenCV REQUIRED)
add_library(grabber src/grabber.cpp)
target_include_directories(grabber PUBLIC ${PROJECT_SOURCE_DIR}/include )


include_directories(/opt/pylon5/include)
link_directories(/opt/pylon5/lib64)
target_link_libraries(grabber ${OpenCV_LIBS} pylonutility pylonbase GCBase_gcc_v3_0_Basler_pylon_v5_0)


#add_executable(basler_test openCVGrab.cpp)
#target_include_directories(basler_test /opt/pylon5/include)
#target_link_libraries(basler_test ${OpenCV_LIBS} pylonutility pylonbase GCBase_gcc_v3_0_Basler_pylon_v5_0 )

#add_executable( main src/main.cpp)

add_executable( calibrator test/grabberTest.cpp)

target_link_libraries(calibrator grabber)

#link executable with library

#target_link_libraries(main grabber)
