CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(VERSION_MAJOR 0)
SET(VERSION "${VERSION_MAJOR}.1.0")

PROJECT(image-viewer C CXX)

EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_VERSION)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

SET(SRCS
# Entry point
	src/ivug-base.cpp

# View related
	src/view/ivug-main-view.cpp
	src/view/ivug-main-view-toolbar.cpp
	src/view/ivug-main-view-menu.cpp
	src/view/ivug-slideshow-view.cpp
	src/view/ivug-name-view.c
	src/view/ivug-photocam.cpp

# Slider
	src/slider/ivug-slider-new.cpp

# Control related
	src/control/ivug-parameter.c
	src/control/ivug-crop-ug.cpp
	src/control/ivug-ext-ug.c

	src/control/ivug-context.c

# Popup
	src/popup/ivug-popup.c

# Thumb list
	src/thumblist/ivug-thumblist.cpp

	src/decolayer/ivug-decolayer.cpp
)

# message("System = ${CMAKE_SYSTEM_VERSION}")

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/include
	${IVUG_SOURCE_WIDGET_DIR}/include
	${IVUG_SOURCE_COMMON_DIR}/include
	${IVUG_SOURCE_MEDIA_DIR}/include
	${IVUG_SOURCE_RESOURCE_DIR}/include
	src
	src/include
	src/control
	src/thumblist
	src/decolayer
	src/popup
	${IVUG_SOURCE_FEATURE_DIR}/include
	${IVUG_SOURCE_SLIDESHOW_DIR}/include

# Below include directory should be removed for make module more indepentatly
#	${IVUG_SOURCE_SLIDESHOW_DIR}/src/effect/include
#	${IVUG_SOURCE_SLIDESHOW_DIR}/src/dali_effect/include
)

INCLUDE(FindPkgConfig)
pkg_check_modules(${PROJECT_NAME}_pkgs REQUIRED
	elementary
	edje
	evas
	eina
	dlog
	ecore
	ecore-imf
	ecore-input
	ecore-evas
	ui-gadget-1
	libmedia-utils
	media-thumbnail
	glib-2.0
	shortcut
	notification
	deviced
	capi-appfw-application
	capi-appfw-app-manager
	capi-media-player
	capi-content-mime-type
	capi-content-media-content
	pkgmgr-info
	sensor
	ui-gadget-1
	efl-extension
	storage
	capi-system-device
)

FOREACH(flag ${${PROJECT_NAME}_pkgs_CFLAGS})
	SET(EXTRA_FLAGS "${EXTRA_FLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS("-DEXPORT_API=__attribute__ \(\(visibility\(\"default\"\)\)\)")
#ADD_DEFINITIONS("-DPACKAGE=\"${PROJECT_NAME}\"")

#ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
#ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${VERSION})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${EXTRA_FLAGS})


TARGET_LINK_LIBRARIES(${PROJECT_NAME}
	${${PROJECT_NAME}_pkgs_LDFLAGS}
	-L${CMAKE_SOURCE_DIR}/Qphoto
	ivug_common ivug_widget ivug_medialist ivug_slideshow
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

SET(THEME_FOLDER images/theme)
SET(THEME white)

# Build edc files
BUILD_EDJ(ivug-base.edc 			ivug-base.edj 			layout		images)
BUILD_EDJ(ivug-best.edc 			ivug-best.edj 			layout		images)
BUILD_EDJ(ivug-buddy-view.edc 		ivug-buddy-view.edj 	layout		images)
BUILD_EDJ(ivug-tag-buddy.edc 		ivug-tag-buddy.edj 		layout		images)
BUILD_EDJ(ivug-popup.edc			ivug-popup.edj 			layout		images)
BUILD_EDJ(ivug-nearby-device.edc	ivug-nearby-device.edj 	layout		images)
BUILD_EDJ(ivug-crop-view.edc		ivug-crop-view.edj 		layout		crop-view)
BUILD_EDJ(ivug-icons.edc 			ivug-icons.edj 			layout		icons)
BUILD_EDJ(ivug-slider-new.edc 		ivug-slider-new.edj 	layout		icons)
BUILD_EDJ(ivug-slider-new-hd.edc 	ivug-slider-new-hd.edj 	layout		icons)
BUILD_EDJ(ivug-setas.edc 			ivug-setas.edj 			layout		${THEME_FOLDER}/${THEME})
BUILD_EDJ(ivug-name-view.edc 		ivug-name-view.edj 		layout		${THEME_FOLDER}/${THEME})
BUILD_EDJ(ivug-edit-weather-view.edc 		ivug-edit-weather-view.edj 		layout		${THEME_FOLDER}/${THEME})
BUILD_EDJ(weather-tag-buddy.edc 		weather-tag-buddy.edj 		layout		${THEME_FOLDER}/${THEME})

BUILD_EDJ_SOUND(ivug-main.edc 		ivug-main.edj 			layout		images		sounds)
BUILD_EDJ_SOUND(ivug-details.edc 	ivug-details.edj 		layout		images		sounds)
BUILD_EDJ_SOUND(ivug-thumblist.edc 	ivug-thumblist.edj 		layout		thumblist	sounds)
BUILD_EDJ_SOUND(ivug-button.edc 	ivug-button.edj 		layout 		buttons 	sounds)




# Install image files.
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/images/ DESTINATION ${CMAKE_INSTALL_PREFIX}/res/images/${IVUG_PACKAGE_NAME})

#
INSTALL(DIRECTORY DESTINATION ${CMAKE_DATA_DIR}/${IVUG_PACKAGE_NAME})



