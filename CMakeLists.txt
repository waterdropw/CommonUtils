cmake_minimum_required(VERSION 3.0)


if(ANDROID)
  add_compile_definitions(_ANDROID_PLATFORM_)
elseif(APPLE)
  add_compile_definitions(_OSX_PLATFORM_)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_compile_definitions(_LINUX_PLATFORM_)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  add_compile_definitions(_WINDOWS_PLATFORM_)
endif()
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    add_compile_definitions(_32_BIT_)
elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    add_compile_definitions(_64_BIT_)
endif()

add_library(utils STATIC 
    src/utils.cpp
    src/os.cpp
)
add_library(COMMON::utils ALIAS utils)

target_compile_features(utils PRIVATE cxx_std_11)
target_compile_options(utils PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

target_include_directories(utils
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if( ANDROID) 
    target_link_libraries(utils PUBLIC log z)
endif()
