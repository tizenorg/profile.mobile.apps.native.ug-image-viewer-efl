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

#ifndef __IVUG_DATATYPES_H__
#define __IVUG_DATATYPES_H__

#include "ivug-define.h"
#include <stdbool.h>

#ifdef _USE_MEDIAINFO_STRINGID_
	typedef const char *UUID;

	#define INVALID_UUID ((UUID)NULL)
#else
	typedef int UUID;

	#define INVALID_UUID ((UUID)-1)
#endif

typedef enum {
IVUG_VIEW_BY_ALL = 0x00,
	//IVUG_VIEW_BY_INVAILD = 0x00,
	IVUG_VIEW_BY_FILE,					// Single file.
//	IVUG_VIEW_BY_ALL,
IVUG_VIEW_BY_INVAILD,
	IVUG_VIEW_BY_HIDDEN_ALL,
	IVUG_VIEW_BY_FAVORITES,
	IVUG_VIEW_BY_TAG,
	IVUG_VIEW_BY_FOLDER,		// Same as DB Album. will be renamed to BY_ALBUM
	IVUG_VIEW_BY_HIDDEN_FOLDER,
	IVUG_VIEW_BY_PLACES,
	IVUG_VIEW_BY_TIMELINE,
	IVUG_VIEW_BY_DIRECTORY,
} ivug_view_by;

typedef enum {
	IVUG_MEDIA_TYPE_ALL = 0x00, //default value
	IVUG_MEDIA_TYPE_IMAGE,
	IVUG_MEDIA_TYPE_VIDEO,
	IVUG_MEDIA_TYPE_MAX,
} ivug_media_type;


typedef enum {
	IVUG_MEDIA_SORT_NONE = 0x00,  /**< No sort */
	IVUG_MEDIA_ASC_BY_NAME,     	/**< Ascending sort as file name */
	IVUG_MEDIA_DESC_BY_NAME,   	/**< Descending sort as file name */
	IVUG_MEDIA_ASC_BY_DATE,     	/**< Ascending sort as created date */
	IVUG_MEDIA_DESC_BY_DATE,    	/**< Descending sort as created date */
} ivug_sort_type;


#endif 		// __IVUG_DATATYPES_H__

