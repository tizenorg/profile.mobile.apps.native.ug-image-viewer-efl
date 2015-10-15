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

#ifndef __IVUG_MEDIA_H__
#define __IVUG_MEDIA_H__

#include "ivug-define.h"
#include "ivug-datatypes.h"
#include "ivug-db.h"
#include "ivug-filter.h"

//#include "ivug-medialist.h"

#include <Eina.h>
#include <stdbool.h>

//slide type
typedef enum {
	SLIDE_TYPE_NONE = 0x00,
	SLIDE_TYPE_IMAGE,
	SLIDE_TYPE_VIDEO,
	SLIDE_TYPE_UNKNOWN,		// File is exists but it's type is unknown.
} Media_Type;

// Meida_Data can be one in File, DB, PTP, DLNA

typedef enum {
	MIMAGE_TYPE_NORMAL,
	MIMAGE_TYPE_SOUNDSCENE,
	MIMAGE_TYPE_PANORAMA,
	MIMAGE_TYPE_ANIMATED,
	MIMAGE_TYPE_BESTSHOT,
} MImageType;

typedef enum {
	DATA_STATE_NONE,
	DATA_STATE_READY,
	DATA_STATE_WAIT,
	DATA_STATE_LOADING,
	DATA_STATE_LOADED,
	DATA_STATE_NO_PERMISSION,
	DATA_STATE_ERROR,
	DATA_STATE_MAX
}Data_State;

typedef struct _Media_Data Media_Data;

/* Opaque pointer for media list. */
typedef void Media_List;

typedef void (*mdata_callback_t)(Media_Data *mdata, Data_State state, void *data);

/*
	This struct represents data for slide.
	thumbnail_path is slide's thumbnail image path.
	fileurl is saved item in media service.
	filepath is local file path. if fileurl is http://.../test.jpg, filepath is NULL, when complete download filepath change to saved local file path.
*/
struct _Media_Data{
	media_handle m_handle;

	int index;

//Data
	Media_Type slide_type;			//image, video, web image.

	UUID mediaID;					// Unique Media ID

	char* thumbnail_path;			// thumbnail image file path.

	char* fileurl;					// file url.
	char* filepath;					// file path in local file system.

	Ivug_DB_h *thumb_handle;

	MImageType iType;			// Only available for image

	Media_List *p_mList;			//parent media list

	Data_State state;
	Data_State thumbnail_state;

	mdata_callback_t thumbnail_callback;
	void *thumb_cb_data;

	mdata_callback_t file_callback;
	void *cb_data;
};

#ifdef __cplusplus
extern "C" {
#endif

bool ivug_mediadata_set_tag(Media_Data *data, const char *newtag);
bool ivug_mediadata_set_favorite(Media_Data *data, bool bFavorite);

bool ivug_mediadata_get_favorite(Media_Data *data, bool *bFavorite);
bool ivug_mediadata_delete(Media_Data * mdata);

bool ivug_mediadata_free(Media_Data * mdata);

void ivug_mediadata_request_thumbnail(Media_Data *mdata, mdata_callback_t callback, void *data);

void ivug_mediadata_request_file(Media_Data *mdata, mdata_callback_t callback, void *data);
void ivug_mediadata_cancel_request_file(Media_Data *mdata);

const char* ivug_mediadata_get_filepath(Media_Data *mdata);
const char* ivug_mediadata_get_fileurl(Media_Data *mdata);
const char* ivug_mediadata_get_thumbnailpath(Media_Data *mdata);

void ivug_mediadata_set_file_state(Media_Data *mdata, Data_State state);

Data_State ivug_mediadata_get_file_state(Media_Data *mdata);
Data_State ivug_mediadata_get_thumbnail_state(Media_Data *mdata);

#ifdef __cplusplus
}
#endif

#endif // __IVUG_MEDIA_H__
