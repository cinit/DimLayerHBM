cmake_minimum_required(VERSION 3.10)

project(disptunerd)

set(CMAKE_CXX_STANDARD 17)

add_executable(disptunerd startup/startup.c startup/daemon.cpp ipc/IpcStateController.cpp ipc/logbuffer/LocalLogBuffer.cpp
        service/front/DispTunerDaemonImpl.cpp service/front/AppClientProxy.cpp service/HwServiceStatus.cpp
        elfsym/ElfView.cpp elfsym/ProcessView.cpp inject/Injector.cpp inject/arch/ptrace_inject_utils.cpp
        inject/arch/ptrace_inject_arm_impl.cpp inject/arch/ptrace_inject_x86_impl.cpp inject/SysServicePatch.cpp
        service/ServiceManager.cpp service/hw/BaseHwHalHandler.cpp
        service/hw/opdispfeature/OpDispFeatureHandler.cpp)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CLANG_CXX_EXTRA_OPT "-Werror=unknown-warning-option -Werror=format-invalid-specifier -Werror=call-to-pure-virtual-from-ctor-dtor")
    SET(CLANG_C_EXTRA_OPT "-Werror=format-invalid-specifier")
else ()
    SET(CLANG_CXX_EXTRA_OPT "")
    SET(CLANG_C_EXTRA_OPT "")
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_CXX_EXTRA_OPT} -fPIC -Werror=delete-non-virtual-dtor -Werror=return-type -Werror=non-virtual-dtor -Wno-invalid-offsetof")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANG_C_EXTRA_OPT} -fPIC -Werror=return-type")

set_target_properties(disptunerd PROPERTIES
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        )

set_target_properties(disptunerd PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"
        )

include_directories(..)

target_compile_definitions(disptunerd PRIVATE DISPLAY_TUNER_VERSION=\"${DISPLAY_TUNER_VERSION}\")

target_link_libraries(disptunerd c dl rpcprotocol ${ANDROID_LIBS})
