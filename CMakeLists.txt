cmake_minimum_required(VERSION 3.12)
project(wibmod VERSION 1.3.2)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(ers REQUIRED)
#find_package(serialization REQUIRED)
find_package(logging REQUIRED)
find_package(appfwk REQUIRED)
find_package(cppzmq REQUIRED)
find_package(Protobuf REQUIRED)

# Build the protobuf source/header without protobuf_generate_cpp because we 
# don't like CMakeList.txt in sub directories...
# FIXME daq buildtools has started caching things and doesn't actually run this every time

include_directories(${PROTOBUF_INCLUDE_DIR})
execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/src --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/wib.proto)

# appfwk magic functions

daq_add_library( wib.pb *.cpp WIB1/*.cpp WIB1/BUException/*.cpp LINK_LIBRARIES ${Protobuf_LIBRARY} cppzmq appfwk::appfwk logging::logging ers::ers )

daq_codegen( *.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )

daq_add_plugin( WIBConfigurator duneDAQModule LINK_LIBRARIES wibmod )

daq_add_plugin( ProtoWIBConfigurator duneDAQModule LINK_LIBRARIES wibmod )

daq_install()
