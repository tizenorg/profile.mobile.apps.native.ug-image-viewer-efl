CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ivug_feature C CXX)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION 0.1.0)

SET(SRCS
	src/ivug-player.cpp
	src/ivug-motion.cpp
	src/ivug-vibration.c
	src/ivug-transcoder.cpp

	src/exif/jpeg.cpp

	src/ivug-comment.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include
	${IVUG_SOURCE_COMMON_DIR}/include
	${IVUG_SOURCE_RESOURCE_DIR}/include
	include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(${PROJECT_NAME}_pkgs REQUIRED
	evas
	ecore
	#ecore-x
	elementary
	dlog
	ui-gadget-1
	deviced
	#gstreamer-0.10
	capi-system-sensor
	capi-media-player
	capi-appfw-app-manager
	capi-media-image-util
	capi-content-media-content
)

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_FLAGS "${EXTRA_FLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DLOG_LVL=DBG_MSG_LVL_HIGH")
ADD_DEFINITIONS("-DLOG_CAT=\"IV-FEATURE\"")

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${EXTRA_FLAGS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_pkgs_LDFLAGS}
	ivug_common
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)

