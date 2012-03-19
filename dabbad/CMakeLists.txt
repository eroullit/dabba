PROJECT(dabbad C)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/include
                        ${CMAKE_SOURCE_DIR}/dabbacore/include)

ADD_EXECUTABLE(dabbad dabbad.c list.c ipc.c help.c capture.c)

TARGET_LINK_LIBRARIES (dabbad dabbacore ${CMAKE_THREAD_LIBS_INIT})

ADD_CUSTOM_TARGET(setcap-dabbad COMMAND ${SETCAP_EXECUTABLE} cap_net_raw,cap_ipc_lock,cap_net_admin=eip	${CMAKE_CURRENT_BINARY_DIR}/dabbad)
ADD_DEPENDENCIES(setcap setcap-dabbad)
