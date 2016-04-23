/*
* Copyright (c) 2000-2015  Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

/*
	ONLY system widely used definition.
*/

#ifndef __IVUG_DEFINE_H__
#define __IVUG_DEFINE_H__

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
# use string UUID
ADD_DEFINITIONS("-D_USE_MEDIAINFO_STRINGID_")
*/
#define _USE_MEDIAINFO_STRINGID_

#define USE_RESCAN_FILE_PATH_AT_LIST

#define IVUG_MAX_FILE_PATH_LEN (4096)

#define DIR_MASK_DEFAULT 0775

#define USE_CUSTOM_STYLE

#define IVUG_INVALID_INDEX (0)				// index was not set

#define MAX_CHAR_LEN 50

#define MAX_BYTE_LEN 254

/*
	Time analysis data will be stored as file
*/
#define TA_SAVETO_FILE

#define USE_NEW_DB_API
#define USE_THUMBLIST
//#define USE_EXT_SLIDESHOW
#define USE_VIDEOCAM

#define USE_ADD_COMMENT

//#define	USE_HELP_POPUP

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " #x))
#define FIXME(x) DO_PRAGMA(message ("FIXME - " #x))

#define INVALID_FILENAME_CHAR "\\/;:*?<>|\""

//#define USE_BROWSE_FUNC	//allshare browse



/************************************************************************************
 *
 *  GUI Configuration
 *
************************************************************************************/


/*
	What kind of theme used.
*/
#define WHITE_THEME


/*
	If enabled, use RECT instead of SPACER
*/
#undef USE_PADDING_RECT

#define USE_SINGLE_ASF	//single file at allshare

#define USE_LIST_RELOAD

#define QPHOTO_CONFIG_DOUBLE_TAP_INTERVAL (0.33f)


/*
	QPHOTO_CONFIG_BETWEEN_IMAGE_GAP
		Gap between images is 60 pixel

	QPHOTO_CONFIG_ANI_SLIDE_MAX_DURATION
	QPHOTO_CONFIG_ANI_SLIDE_MIN_DURATION


	QPHOTO_CONFIG_ANI_SNAP_BACK_DURATION
		Image
*/

#define QPHOTO_CONFIG_ANI_SLIDE_MAX_DURATION (300)
#define QPHOTO_CONFIG_ANI_SLIDE_MIN_DURATION (5)
#define QPHOTO_CONFIG_ANI_SLIDE_INVERSE_PROPORTION (2000000)

#define QPHOTO_CONFIG_ANI_SNAP_BACK_DURATION (400)
#define QPHOTO_CONFIG_ANI_ZOOM_DURATION (50)

#define QPHOTO_CONFIG_ANI_DOUBLE_TAB_ZOOM_DURATION (334)

#define QPHOTO_CONFIG_BETWEEN_IMAGE_GAP (60)

#endif 	// __IVUG_DEFINE_H__
