FILE(GLOB sharness sharness.sh)
FILE(GLOB test_scripts t*.sh)

EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${sharness} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/test)

FOREACH(test_script ${test_scripts})
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy ${test_script} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/test)
	ADD_TEST(${test_script} sh ${test_script})
ENDFOREACH(test_script ${test_scripts})