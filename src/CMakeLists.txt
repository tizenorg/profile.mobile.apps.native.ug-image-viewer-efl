CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(image-viewer C CXX)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(LIBDIR "${PREFIX}/lib")

SET(VERSION 0.1.0)

SET(SRCS
	ug-image-viewer.cpp
	common/debug.c
	common/ivug-config.c
	common/ivug-dir.cpp
	common/ivug-file-info.c
	common/ivug-language-mgr.c
	common/ivug-util.c
	common/statistics.c
	common/ivug-callback.c
	common/ivug-db.c
	common/ivug-exif.c
	common/ivug-file-util.c
	common/ivug-message.c
	common/ivug-uuid.c
	#feature/ivug-comment.cpp
	#feature/ivug-motion.cpp
	#feature/ivug-transcoder.cpp
	#feature/ivug-vibration.c
	#feature/jpeg.cpp
	main/ivug-base.cpp
	main/control/ivug-context.c
	main/control/ivug-crop-ug.cpp
	main/control/ivug-ext-ug.c
	main/control/ivug-parameter.c
	main/decolayer/iv-button.cpp
	main/decolayer/ivug-decolayer.cpp
	main/popup/ivug-popup.c
	main/slider/ivug-slider-new.cpp
	main/thumblist/ivug-thumblist.cpp
	main/view/ivug-main-view.cpp
	main/view/ivug-main-view-toolbar.cpp
	main/view/ivug-photocam.cpp
	main/view/ivug-main-view-menu.cpp
	main/view/ivug-name-view.c
	main/view/ivug-slideshow-view.cpp
	medialist/ivug-data-list.c
	medialist/ivug-filter.c
	medialist/ivug-mediadata.c
	medialist/ivug-mediainfo.cpp
	medialist/ivug-medialist.cpp
	slideshow/control/ivug-slideshow.cpp
	slideshow/effect/ivug-anim.c
	slideshow/effect/ivug-effect.c
	slideshow/effect/ivug-fade.c
	slideshow/effect/ivug-slide.c
	#slideshow/effect/template.c
)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/inc/)



INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	elementary
	capi-appfw-application
	capi-appfw-app-manager
	appsvc
	efl-extension
	capi-system-runtime-info
	capi-system-system-settings
	libtzplatform-config
	edje
	evas
	eina
	dlog
	ecore
	ecore-imf
	ecore-input
	ecore-evas
	libmedia-utils
	media-thumbnail
	glib-2.0
	shortcut
	notification
	deviced
	capi-media-player
	capi-content-mime-type
	capi-content-media-content
	pkgmgr-info
	sensor
	storage
	capi-system-device
	capi-system-sensor
	capi-media-image-util
	capi-media-sound-manager
	ecore-file
	efreet-mime
	icu-i18n
	capi-media-metadata-extractor
	capi-appfw-preference
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -fvisibility=hidden -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

ADD_DEFINITIONS("-DUG_MODULE_API=__attribute__ \(\(visibility\(\"default\"\)\)\)")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DDATADIR=\"${CMAKE_DATA_DIR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PROJECT_NAME}\"")
ADD_DEFINITIONS("-DIVAPI=")

#Set executable
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

# install binary
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})
