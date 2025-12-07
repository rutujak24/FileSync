cmake_minimum_required(VERSION 3.15)
project(FileSyncPlusPlus CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Find Protobuf compiler and gRPC plugin
find_program(Protobuf_PROTOC_EXECUTABLE protoc)
find_program(gRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

if(NOT Protobuf_PROTOC_EXECUTABLE OR NOT gRPC_CPP_PLUGIN_EXECUTABLE)
    message(FATAL_ERROR "Could not find protoc or grpc_cpp_plugin")
endif()

# Protobuf/gRPC generation
set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protos)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/protos)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# Helper to generate protos
function(generate_protos PROTO_FILE SERVICE_NAME)
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(PROTO_HDRS "${PROTO_GEN_DIR}/${PROTO_NAME}.pb.h" "${PROTO_GEN_DIR}/${PROTO_NAME}.grpc.pb.h")
    set(PROTO_SRCS "${PROTO_GEN_DIR}/${PROTO_NAME}.pb.cc" "${PROTO_GEN_DIR}/${PROTO_NAME}.grpc.pb.cc")

    add_custom_command(
        OUTPUT ${PROTO_HDRS} ${PROTO_SRCS}
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS --grpc_out=${PROTO_GEN_DIR}
             --cpp_out=${PROTO_GEN_DIR}
             --plugin=protoc-gen-grpc=${gRPC_CPP_PLUGIN_EXECUTABLE}
             -I ${PROTO_SRC_DIR}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
    )

    add_library(${SERVICE_NAME}_proto ${PROTO_SRCS} ${PROTO_HDRS})
    target_link_libraries(${SERVICE_NAME}_proto PUBLIC ${GRPC_LIBRARIES} ${PROTOBUF_LIBRARIES})
    target_include_directories(${SERVICE_NAME}_proto PUBLIC ${PROTO_GEN_DIR} ${GRPC_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})
endfunction()

generate_protos(${PROTO_SRC_DIR}/filesync.proto filesync)
generate_protos(${PROTO_SRC_DIR}/crdt.proto crdt)

# Include directories
include_directories(include)
include_directories(${PROTO_GEN_DIR})
include_directories(${GRPC_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})

# Server
add_executable(filesync_server src/server/main.cpp src/server/server.cpp src/db/db_manager.cpp src/common/utils.cpp src/common/crdt_manager.cpp)
target_link_libraries(filesync_server PRIVATE filesync_proto crdt_proto SQLite::SQLite3 OpenSSL::SSL OpenSSL::Crypto ${GRPC_LIBRARIES} ${PROTOBUF_LIBRARIES})

# Client
add_executable(filesync_client src/client/main.cpp src/client/client.cpp src/common/utils.cpp src/common/crdt_manager.cpp)
target_link_libraries(filesync_client PRIVATE filesync_proto crdt_proto OpenSSL::SSL OpenSSL::Crypto ${GRPC_LIBRARIES} ${PROTOBUF_LIBRARIES})
