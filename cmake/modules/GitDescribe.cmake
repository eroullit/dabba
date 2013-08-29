#
# Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
#

# Generate man pages of the project by using the
# POD header written in the tool source code.
# To use it, include this file in CMakeLists.txt and
# invoke GIT_DESCRIBE(<var>)
# Returns a git description string or N/A when not available

FUNCTION(GIT_DESCRIBE VAR)
	IF(NOT GIT_FOUND)
		FIND_PACKAGE(Git QUIET)
	ENDIF(NOT GIT_FOUND)

	IF(GIT_FOUND)
		EXECUTE_PROCESS(COMMAND "${GIT_EXECUTABLE}" describe --tags
				RESULT_VARIABLE RC OUTPUT_VARIABLE OUT
				ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
		IF(RC EQUAL 0)
			SET(${VAR} "${OUT}" PARENT_SCOPE)
		ENDIF(RC EQUAL 0)
	ENDIF(GIT_FOUND)

	IF(NOT DEFINED VAR)
		SET(${VAR} "N/A" PARENT_SCOPE)
	ENDIF(NOT DEFINED VAR)
ENDFUNCTION(GIT_DESCRIBE VAR)
