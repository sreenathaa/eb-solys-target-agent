
SET (CMAKE_VERBOSE_MAKEFILE off)

cmake_minimum_required(VERSION 2.8.5)
SET (PLUGIN_PROVIDER_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (PLUGIN_PROVIDER_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/inc)

find_package(Poco REQUIRED Foundation Util Net XML JSON)

# Source files
SET (PLUGIN_PROVIDER_LIB_SRC 	${PLUGIN_PROVIDER_SOURCE_PATH}/plugin_provider.cpp
								${PLUGIN_PROVIDER_SOURCE_PATH}/protocol_message.cpp
								${PLUGIN_PROVIDER_SOURCE_PATH}/CTimestampProvider.cpp)
				
ADD_LIBRARY(plugin-provider STATIC ${PLUGIN_PROVIDER_LIB_SRC} ${PLUGIN_PROVIDER_LIB_INC})

TARGET_LINK_LIBRARIES(plugin-provider 
    ${PROTO_BUF_LIB} 
    Poco::Foundation
    Poco::Util
    Poco::Net
    Poco::XML
    Poco::JSON
    )


target_include_directories(plugin-provider
    PUBLIC
        $<BUILD_INTERFACE:
					 ${CMAKE_CURRENT_SOURCE_DIR}/inc:
					 ${PLUGIN_PROVIDER_INCLUDE_PATH}
					 ${PROJECT_SOURCE_DIR}/config-provider/inc
					 ${PROJECT_SOURCE_DIR}/message-recorder/inc
					 ${CMAKE_CURRENT_SOURCE_DIR}/../communicator/inc
					 ${CMAKE_CURRENT_SOURCE_DIR}/../communicator/gen
					 ${CMAKE_CURRENT_SOURCE_DIR}/../logger/inc
					 ${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/gen
		>
        $<INSTALL_INTERFACE:inc>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Timestamp Provider tests
IF(${TA_LOCAL_ENABLE_TESTING_DISABLED})
    
	IF (CMAKE_COMPILER_IS_GNUCC)
		SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
		SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fprofile-arcs -ftest-coverage")
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
	ENDIF()
	INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/inc
						 ${CMAKE_CURRENT_SOURCE_DIR}/../logger/test/
						 ${CMAKE_CURRENT_SOURCE_DIR}/../target-agent-runtime/gen
						 )
	SET (TIMESTAMP_PROVIDER_TEST_LIB_SRC	${CMAKE_CURRENT_SOURCE_DIR}/test/CTimestampProviderTests.cpp
										    ${CMAKE_CURRENT_SOURCE_DIR}/src/CTimestampProvider.cpp
										     ${CMAKE_CURRENT_SOURCE_DIR}/../logger/test/config_mock.cpp
										)

	ADD_EXECUTABLE(timestamp_provider-test ${TIMESTAMP_PROVIDER_TEST_LIB_SRC})
	
	TARGET_LINK_LIBRARIES(timestamp_provider-test ${GTEST_MAIN_LIB} 
            ${GTEST_LIB} 
            ${GMOCK_LIB}
            ${ADDITIONAL_LIBRARY} 														
            Poco::Foundation
            Poco::Util
            Poco::Net
            Poco::XML
            Poco::JSON
            )

	add_test(
		NAME timestamp_provider-test
		COMMAND timestamp_provider-test
	)
						 
	
	SET (PROTOCOL_MSG_TEST_LIB_SRC	        ${CMAKE_CURRENT_SOURCE_DIR}/test/protocol_message_test.cpp
										    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol_message.cpp
										    ${CMAKE_CURRENT_SOURCE_DIR}/../logger/test/config_mock.cpp
										)

	ADD_EXECUTABLE(protocol_message${DEBUG_SUFFIX}-test ${PROTOCOL_MSG_TEST_LIB_SRC})
	
	TARGET_LINK_LIBRARIES(protocol_message${DEBUG_SUFFIX}-test ${GTEST_MAIN_LIB} ${GTEST_LIB} ${GMOCK_LIB}		
            Poco::Foundation
            Poco::Util
            Poco::Net
            Poco::XML
            Poco::JSON
            )

	add_test(
		NAME protocol_message${DEBUG_SUFFIX}-test
		COMMAND protocol_message${DEBUG_SUFFIX}-test
	)
	
ENDIF()
				


