cmake_minimum_required(VERSION 3.12)
project(wibmod VERSION 1.0.0)

find_package(daq-cmake REQUIRED)
daq_setup_environment()

find_package(appfwk REQUIRED)

# Stolen from DUNE-DAQ/ipm to find ZMQ with the curious ENV vars from `setup`
if(EXISTS $ENV{ZMQ_LIB})
    message("Finding CPPZMQ from UPS $ENV{ZMQ_LIB}")
    include_directories($ENV{ZMQ_INC})
    find_library(ZMQ NAMES libzmq.so HINTS $ENV{ZMQ_LIB})
    message("find_library ZMQ found ${ZMQ}")
    set(cppzmq_FOUND TRUE)
else() # fallback
	find_package(cppzmq REQUIRED)
	set(ZMQ zmq)
endif()

# Build the protobuf source/header without protobuf_generate_cpp because we 
# don't like CMakeList.txt in sub directories...
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/src --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/wib.proto)

# appfwk magic functions
daq_add_library( wib.pb WIBCommon LINK_LIBRARIES ${ZMQ} appfwk::appfwk)
daq_add_plugin( WIBConfigurator duneDAQModule SCHEMA LINK_LIBRARIES wibmod appfwk::appfwk)
daq_install()
