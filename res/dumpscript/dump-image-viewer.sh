IMAGEVIEWER_DEBUG=$1/image-viewer
GALLERY_DEBUG=/opt/usr/apps/com.samsung.gallery/data


IMAGEVIEWER_DATA_DIR="/opt/usr/ug/data/ug-image-viewer-efl/.debug"

/bin/mkdir -p ${IMAGEVIEWER_DEBUG}

#xinfo -p 2> ${IMAGEVIEWER_DEBUG}/ping.log
#xinfo -xwd_topvwins ${IMAGEVIEWER_DEBUG}

# copy TimeAnal
/bin/cp -arf ${IMAGEVIEWER_DATA_DIR}/* ${IMAGEVIEWER_DEBUG}

# copy gallery media db for debugging
/bin/cp -arf ${GALLERY_DEBUG}/.gallery* ${IMAGEVIEWER_DEBUG}