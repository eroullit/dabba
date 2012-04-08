FILE(GLOB sharness sharness.sh dabba-test-lib.sh)
FILE(GLOB test_scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} t*.sh)

FOREACH(sharness_file ${sharness})
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${sharness_file} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/test)
ENDFOREACH(sharness_file ${sharness})

FOREACH(test_script ${test_scripts})
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${test_script} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/test)
	ADD_TEST(${test_script} sh ${test_script})
ENDFOREACH(test_script ${test_scripts})
