

SET (CMAKE_VERBOSE_MAKEFILE off)

cmake_minimum_required(VERSION 2.8.5)
SET (COMMUNICATOR_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (COMMUNICATOR_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc)
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(ENV{LD_LIBRARY_PATH} ${PROTO_PATH_TO_GENERATOR})

include(FindProtobuf)
find_package(Protobuf REQUIRED)
find_package(Poco REQUIRED Foundation Util Net XML JSON)

INCLUDE_DIRECTORIES (${COMMUNICATOR_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/gen
    ${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/gen
)

# Source files
SET (COMMUNICATOR_LIB_SRC ${COMMUNICATOR_SOURCE_PATH}/CComController.cpp
    ${COMMUNICATOR_SOURCE_PATH}/CComServer.cpp
    ${COMMUNICATOR_SOURCE_PATH}/CMessageDispatcher.cpp	   
    ${COMMUNICATOR_SOURCE_PATH}/CommandControlServer.cpp
)

# protobuf-related actions
SET (PROTOCOL_DEFINITION_SOURCE_DIR 	${PROJECT_SOURCE_DIR}/../../protocol-definitions)
SET (PROTOCOL_DEFINITION_INPUT_FILES 	${PROTOCOL_DEFINITION_SOURCE_DIR}/target_agent_prot_frame.proto
										${PROTOCOL_DEFINITION_SOURCE_DIR}/target_agent_prot_ctrl.proto
										${PROTOCOL_DEFINITION_SOURCE_DIR}/target_agent_prot_common_definitions.proto)				    

SET (PROTOBUF_GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen)

if (${PROTO_REMOVE_GEN_FILES})
    file (GLOB remove_file "${CMAKE_CURRENT_SOURCE_DIR}/gen/*.*")
    list(LENGTH remove_file RES_LEN)
        if(NOT ${RES_LEN} EQUAL "0")
                file (REMOVE ${remove_file})
        endif()
endif()

foreach(PROTO_FILE ${PROTOCOL_DEFINITION_INPUT_FILES})
	#get the name of the file without extension
	get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

	#add the generated files
	set (PROTO_GEN_SOURCE_FILES ${PROTO_GEN_SOURCE_FILES}
	${PROTOBUF_GEN_OUTPUT_DIR}/${PROTO_NAME}.pb.cc)

	set (PROTO_GEN_HEADER_FILES ${PROTO_GEN_HEADER_FILES}
	${PROTOBUF_GEN_OUTPUT_DIR}/${PROTO_NAME}.pb.h)
	

endforeach(PROTO_FILE ${PROTOCOL_DEFINITION_INPUT_FILES})

execute_process(
        COMMAND ${Protobuf_PROTOC_EXECUTABLE} --proto_path=${PROTOCOL_DEFINITION_SOURCE_DIR} --cpp_out=${PROTOBUF_GEN_OUTPUT_DIR} ${PROTOCOL_DEFINITION_INPUT_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


SET(COMMON_SOURCE_DIR ${COMMUNICATOR_SOURCE_PATH}/../gen/target_agent_prot_common_definitions.pb.h)
SET(COMMON_DEST_DIR ${PROJECT_SOURCE_DIR}/plugin-interface/gen/)

file(COPY ${COMMON_SOURCE_DIR} DESTINATION ${COMMON_DEST_DIR})

ADD_LIBRARY(communicator STATIC ${COMMUNICATOR_LIB_SRC} ${COMMUNICATOR_LIB_INC} ${PROTO_GEN_SOURCE_FILES} ${PROTO_GEN_HEADER_FILES})
TARGET_LINK_LIBRARIES(communicator 
    logger 
    ${PROTOBUF_LIBRARY} 
    Poco::Foundation
    Poco::Util
    Poco::Net
    Poco::XML
    Poco::JSON
    ${ADDITIONAL_LIBRARY}
)

IF(${TA_LOCAL_ENABLE_TESTING})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	IF (CMAKE_COMPILER_IS_GNUCC)
	  SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
	  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
	  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
	ENDIF()

	SET (COMMAND_CONTROL_SERVER_SRC	${CMAKE_CURRENT_SOURCE_DIR}/test/ProtocolMessageMocks.cpp
									${CMAKE_CURRENT_SOURCE_DIR}/test/CommandControlServerTest.cpp
									${CMAKE_CURRENT_SOURCE_DIR}/src/CommandControlServer.cpp
									)

	ADD_EXECUTABLE(command-control-server-test ${COMMAND_CONTROL_SERVER_SRC})
	
	TARGET_LINK_LIBRARIES(command-control-server-test 
            ${GTEST_MAIN_LIB} 
            ${GTEST_LIB} 
            ${GMOCK_LIB}  
            ${ADDITIONAL_LIBRARY} 
            ${PROTO_BUF_LIB}
            Poco::Foundation
            Poco::Util
            Poco::Net
            Poco::XML
            Poco::JSON
            )

	add_test(
		NAME command-control-server-test
		COMMAND command-control-server-test
	)
	
ENDIF()

