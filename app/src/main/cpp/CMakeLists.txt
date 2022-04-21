cmake_minimum_required(VERSION 3.10)

project(disp_tuner_natives)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

find_library( # Defines the name of the path variable that stores the
        # location of the NDK library.
        ANDROID_LIBS
        # Specifies the name of the NDK library that
        # CMake needs to locate.
        log)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    include_directories("/usr/lib/jvm/java-8-openjdk/include")
    include_directories("/usr/lib/jvm/java-8-openjdk/include/linux")
endif ()

add_subdirectory(rpcprotocol)
add_subdirectory(libappclient)
add_subdirectory(disptunerd)
add_subdirectory(libbasehalpatch)
add_subdirectory(libopdispfeatpatch)
