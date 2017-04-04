cmake_minimum_required(VERSION 3.0)


add_subdirectory(poco)


if (WIN32)
    add_subdirectory(protobufSrc/cmake)
else (WIN32)
    
add_subdirectory(protobuf)
       
endif(WIN32)

#add_subdirectory(gtest)
#add_subdirectory(gmock)

