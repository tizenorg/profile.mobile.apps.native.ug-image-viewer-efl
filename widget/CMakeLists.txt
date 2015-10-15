CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ivug_widget C CXX)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION 0.1.0)

SET(SRCS
# Helper files
	src/util/EFLUtil.cpp
	src/util/ivug-listpopup.c
	src/util/ivug-widget.c
	src/common/iv-button.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include
	${IVUG_SOURCE_COMMON_DIR}/include
	${IVUG_SOURCE_FEATURE_DIR}/include
	include
	src/common
)

INCLUDE(FindPkgConfig)
pkg_check_modules(${PROJECT_NAME}_pkgs REQUIRED
	elementary
	ecore
	ecore-evas
	edje
	eina
	evas
	dlog
	cairo
	ui-gadget-1
	efl-extension
)

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_FLAGS "${EXTRA_FLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DLOG_LVL=DBG_MSG_LVL_WARN")
ADD_DEFINITIONS("-DLOG_CAT=\"IV-WIDGET\"")

message("WIDGET FLAGS=" ${CMAKE_CXX_FLAGS})

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${EXTRA_FLAGS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_pkgs_LDFLAGS}
	ivug_common ivug_feature
)

BUILD_EDJ(ivug-widget-icons.edc 	ivug-widget-icons.edj 	res/layout	icons)
BUILD_EDJ(ivug-widget-button.edc 	ivug-widget-button.edj 	res/layout	icons)

#install resouce files.
# Lack of a trailing slash means install this directory, trailing slash means
# install everything in this directory.


INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)

