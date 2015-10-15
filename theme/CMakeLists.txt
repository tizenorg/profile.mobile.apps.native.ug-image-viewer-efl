CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#PROJECT(ivug-theme)

SET(IMAGE_BASE "images")
SET(SOUND_BASE "sounds")


ADD_CUSTOM_TARGET(theme ALL
	COMMAND
		edje_cc
		-id ${IMAGE_BASE}/naviframe
		-id ${IMAGE_BASE}/gengrid
		-id ${IMAGE_BASE}/buttons
		-id ${IMAGE_BASE}/genlist
		-sd ${SOUND_BASE}
		-DDEFINE_H=${IVUG_SOURCE_DEFINE_DIR}
		ivug-custom.edc
		ivug-custom.edj
	COMMENT "Compiling edj"
	DEPENDS
		ivug-custom.edc
)

ADD_DEPENDENCIES(theme ivug-custom.edj)

INSTALL(FILES ivug-custom.edj DESTINATION ${CMAKE_INSTALL_PREFIX}/res/edje/${IVUG_PACKAGE_NAME})


