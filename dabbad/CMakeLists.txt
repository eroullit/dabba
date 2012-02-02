PROJECT(dabbad C)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/include
                        ${CMAKE_SOURCE_DIR}/dabbacore/include)

ADD_EXECUTABLE(dabbad dabbad.c list.c ipc.c)

TARGET_LINK_LIBRARIES (dabbad dabbacore)