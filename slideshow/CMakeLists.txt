CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ivug_slideshow C CXX)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION 0.1.0)

SET(SRCS
# Effect files
	src/control/ivug-slideshow.cpp
	src/effect/ivug-fade.c
	src/effect/ivug-effect.c
	src/effect/ivug-slide.c
	src/effect/ivug-anim.c
)


INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include
	${IVUG_SOURCE_WIDGET_DIR}/include
	${IVUG_SOURCE_COMMON_DIR}/include
	${IVUG_SOURCE_MEDIA_DIR}/include
	${IVUG_SOURCE_FEATURE_DIR}/include
	include
	src/include
	src/control
	src/effect/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(${PROJECT_NAME}_pkgs REQUIRED
	elementary
	ecore
	ecore-evas
	efl-extension
	edje
	eina
	evas
	dlog
	#ecore-x
	#xext
	ui-gadget-1
	capi-appfw-application
	capi-media-player
	capi-media-sound-manager
	capi-system-device
)

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_FLAGS "${EXTRA_FLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DLOG_LVL=DBG_MSG_LVL_HIGH")
ADD_DEFINITIONS("-DLOG_CAT=\"IV-SLIDESHOW\"")

#ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${EXTRA_FLAGS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_pkgs_LDFLAGS}
	ivug_common ivug_medialist ivug_widget ivug_feature
)

#install resouce files.
# Lack of a trailing slash means install this directory, trailing slash means
# install everything in this directory.

BUILD_EDJ(ivug-ss-ly.edc 			ivug-ss-ly.edj 		res/layout	icons)

# Install image files.
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/images/ DESTINATION ${CMAKE_INSTALL_PREFIX}/res/images/${IVUG_PACKAGE_NAME})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)

