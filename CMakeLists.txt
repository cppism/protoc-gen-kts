cmake_minimum_required(VERSION 3.20)

if (DEFINED PROTOBUF_VERSION)
    message(STATUS "Protobuf version: ${PROTOBUF_VERSION}")
else ()
    message(FATAL_ERROR "Protobuf version is not defined.")
endif ()

project(
        protoc-gen-kts
        VERSION ${PROTOBUF_VERSION}
        LANGUAGES CXX
)

string(TOLOWER ${CMAKE_SYSTEM_NAME} PLATFORM)
string(APPEND PLATFORM "-${CMAKE_SYSTEM_PROCESSOR}")

find_package(Protobuf ${PROTOBUF_VERSION} CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/Main.cc)
set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME_DEBUG ${PROJECT_NAME}
        OUTPUT_NAME_RELEASE "${PROJECT_NAME}-${PLATFORM}"
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        protobuf::libprotobuf
        protobuf::libprotoc
)