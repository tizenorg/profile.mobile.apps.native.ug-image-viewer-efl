CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ivug_common C CXX)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION 0.1.0)

SET(SRCS
	src/debug.c
	src/statistics.c
	src/ivug-uuid.c
	src/ivug-util.c
	src/ivug-file-util.c
	src/ivug-file-info.c
	src/ivug-callback.c
	src/ivug-config.c
	src/ivug-message.c
	src/ivug-exif.c
	src/ivug-db.c
	src/ivug-dir.cpp
	src/ivug-language-mgr.c
)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include
	include
	src
)

INCLUDE(FindPkgConfig)
pkg_check_modules(${PROJECT_NAME}_pkgs REQUIRED
	elementary
	evas
	ecore-evas
	ecore-file
	efreet-mime
	dlog
	glib-2.0
	icu-i18n
	capi-media-metadata-extractor
	capi-content-mime-type
	capi-content-media-content
	capi-system-system-settings
	capi-system-runtime-info
	capi-appfw-preference
)

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_FLAGS "${EXTRA_FLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION} )
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${EXTRA_FLAGS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib)

