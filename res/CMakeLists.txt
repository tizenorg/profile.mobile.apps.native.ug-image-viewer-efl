CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INSTALL(DIRECTORY images/ DESTINATION ${CMAKE_INSTALL_PREFIX}/res/images/${IVUG_PACKAGE_NAME})

INSTALL(FILES dumpscript/dump-image-viewer.sh DESTINATION "/opt/etc/dump.d/module.d")
INSTALL(FILES xml/ug-image-viewer-efl_ChangeableColorInfo.xml DESTINATION ${IVUG_DATA_DIR})
INSTALL(FILES xml/ug-image-viewer-efl_ChangeableFontInfo.xml DESTINATION ${IVUG_DATA_DIR})

# Make debug dir
INSTALL(DIRECTORY DESTINATION ${CMAKE_DATA_DIR}/${IVUG_PACKAGE_NAME}/.debug)

ADD_SUBDIRECTORY(po)
