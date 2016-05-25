CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(image-viewer C CXX)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# subdir
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(src/common)
ADD_SUBDIRECTORY(src/feature)
ADD_SUBDIRECTORY(src/main)
ADD_SUBDIRECTORY(src/main/control)
ADD_SUBDIRECTORY(src/main/decolayer)
ADD_SUBDIRECTORY(src/main/popup)
ADD_SUBDIRECTORY(src/main/slider)
ADD_SUBDIRECTORY(src/main/thumblist)
ADD_SUBDIRECTORY(src/main/view)
ADD_SUBDIRECTORY(src/medialist)
ADD_SUBDIRECTORY(src/slideshow)
ADD_SUBDIRECTORY(src/slideshow/control)
ADD_SUBDIRECTORY(src/slideshow/effect)
ADD_SUBDIRECTORY(res/edje)
ADD_SUBDIRECTORY(res/edje/icons)
ADD_SUBDIRECTORY(res/edje/images)
ADD_SUBDIRECTORY(res/po)
ADD_SUBDIRECTORY(res/edje/sounds)

CONFIGURE_FILE(image-viewer.manifest-3.0.in ${PKGNAME}.manifest)

FUNCTION (BUILD_EDJ _in _out)
	ADD_CUSTOM_TARGET(${_out}
			COMMAND
				edje_cc
				-id ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/images
				-id ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/icons
				-sd ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/sounds
				${CMAKE_CURRENT_SOURCE_DIR}/res/edje/${_in}
				${CMAKE_BINARY_DIR}/${_out}
			DEPENDS
				${CMAKE_CURRENT_SOURCE_DIR}/res/edje/${_in}
	)
	ADD_DEPENDENCIES(${PROJECT_NAME} ${_out})
	INSTALL(FILES ${CMAKE_BINARY_DIR}/${_out} DESTINATION ${RESDIR}/edje)
ENDFUNCTION (BUILD_EDJ)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/res/edje)

# Build edc files
BUILD_EDJ(ivug-base.edc ivug-base.edj)
BUILD_EDJ(ivug-best.edc ivug-best.edj)
BUILD_EDJ(ivug-buddy-view.edc ivug-buddy-view.edj)
BUILD_EDJ(ivug-button_new.edc ivug-button_new.edj)
BUILD_EDJ(ivug-crop-view.edc ivug-crop-view.edj)
BUILD_EDJ(ivug-custom.edc ivug-custom.edj)
BUILD_EDJ(ivug-details.edc ivug-details.edj)
BUILD_EDJ(ivug-edit-weather-view.edc ivug-edit-weather-view.edj)
BUILD_EDJ(ivug-effect.edc ivug-effect.edj)
BUILD_EDJ(ivug-face-view.edc ivug-face-view.edj)
BUILD_EDJ(ivug-icons.edc ivug-icons.edj)
BUILD_EDJ(ivug-main.edc ivug-main.edj)
BUILD_EDJ(ivug-name-view.edc ivug-name-view.edj)
BUILD_EDJ(ivug-nearby-device.edc ivug-nearby-device.edj)
BUILD_EDJ(ivug-popup.edc ivug-popup.edj)
BUILD_EDJ(ivug-scroller.edc ivug-scroller.edj)
BUILD_EDJ(ivug-setas.edc ivug-setas.edj)
BUILD_EDJ(ivug-slider-new.edc ivug-slider-new.edj)
BUILD_EDJ(ivug-slider-new-hd.edc ivug-slider-new-hd.edj)
BUILD_EDJ(ivug-slider.edc ivug-slider.edj)
BUILD_EDJ(ivug-ss-ly.edc ivug-ss-ly.edj)
BUILD_EDJ(ivug-tag-buddy.edc ivug-tag-buddy.edj)
BUILD_EDJ(ivug-thumblist.edc ivug-thumblist.edj)
BUILD_EDJ(ivug-widget-button.edc ivug-widget-button.edj)
BUILD_EDJ(ivug-widget-icons.edc ivug-widget-icons.edj)
BUILD_EDJ(weather-tag-buddy.edc weather-tag-buddy.edj)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/images DESTINATION ${RESDIR}/edje)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/icons DESTINATION ${RESDIR}/edje)


INSTALL(FILES image-viewer.manifest.xml DESTINATION ${MANIFESTDIR} RENAME ${PKGNAME}.xml)
#INSTALL(FILES ${CMAKE_BINARY_DIR}/image-viewer.xml DESTINATION ${MANIFESTDIR})
INSTALL(FILES shared/res/org.tizen.image-viewer.png DESTINATION ${ICONDIR})
