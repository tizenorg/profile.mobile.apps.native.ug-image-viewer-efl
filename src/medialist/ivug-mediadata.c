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

#include "ivug-define.h"
#include "ivug-datatypes.h"
#include "ivug-uuid.h"
#include "ivug-debug.h"
#include "ivug-medialist.h"
#include "ivug-mediadata.h"
#include "ivug-util.h"
#include "ivug-file-info.h"
#include "ivug-mediainfo.h"

#include "ivug-db.h"
#include <media_content.h>

#define IMAGE_PATH		PREFIX"/res/images/"
#define DATA_PATH 		DATADIR"/"

#define DEFAULT_THUMBNAIL_PATH		IMAGE_PATH"/T01_Nocontents_broken.png"
#define IVUG_WEB_DOWNLOAD_TEMP_DIR	DATA_PATH

static char *_strerror_db(int error)
{
	switch (error)
	{
	case MEDIA_CONTENT_ERROR_INVALID_PARAMETER:
		return "Invalid parameter";
	case MEDIA_CONTENT_ERROR_OUT_OF_MEMORY :
		return "Out of memory";
	case MEDIA_CONTENT_ERROR_DB_FAILED :
		return "DB operation failed";
	default:
		{
			static char buf[40];
			snprintf(buf, 40, "Error Code=%d", error);
			return buf;
		}

	}
}

static void _ivug_mediadata_call_file_callback(Media_Data *mdata, Data_State state)
{
	IV_ASSERT(mdata != NULL);
	if (mdata->file_callback)
	{
		mdata->state = state;
		MSG_SDATA_HIGH("mdata 0x%08x, mdata->state 0x%08x, mdata->cb_data 0x%08x", mdata, mdata->state, mdata->cb_data);
		mdata->file_callback(mdata, mdata->state, mdata->cb_data);
		mdata->file_callback = NULL;
	}
}

static void _ivug_mediadata_call_thumbnail_callback(Media_Data *mdata, Data_State state)
{
	IV_ASSERT(mdata != NULL);
	if (mdata->thumbnail_callback)
	{
		mdata->thumbnail_state = state;
		MSG_SDATA_HIGH("mdata 0x%08x, mdata->thumbnail_state 0x%08x, mdata->thumb_cb_data 0x%08x", mdata, mdata->thumbnail_state, mdata->thumb_cb_data);
		mdata->thumbnail_callback(mdata, mdata->thumbnail_state, mdata->thumb_cb_data);
		mdata->thumbnail_callback = NULL;
	}
}

Media_Data *ivug_alloc_mediadata_from_media_handle(media_handle media)
{
	IV_ASSERT(media != NULL);

	Media_Data * mdata = NULL;
	mdata = (Media_Data *) calloc(1, sizeof(Media_Data));
	if (mdata == NULL)
	{
		MSG_SDATA_ERROR("Cannot allocate memory");
		return NULL;
	}

	media_info_h item = (media_info_h)media;

	int ret = media_info_clone((media_info_h*)&(mdata->m_handle), item);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_SDATA_ERROR("media_info_foreach_media_from_db is failed, err = %s", _strerror_db(ret));
		goto ALLOC_MHANDLE_ERROR;
	}

	char *uuid = NULL;

	ret = media_info_get_media_id(item, &uuid);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_SDATA_ERROR("media_info_foreach_media_from_db is failed, err = %s", _strerror_db(ret));
		goto ALLOC_MHANDLE_ERROR;
	}

	mdata->mediaID = uuid_assign(uuid);
	free(uuid);

	//file url
	ret = media_info_get_file_path(item, &(mdata->fileurl));
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_SDATA_ERROR("media_info_foreach_media_from_db is failed, err = %s", _strerror_db(ret));
		goto ALLOC_MHANDLE_ERROR;
	}
	if (mdata->fileurl == NULL)
	{
		goto ALLOC_MHANDLE_ERROR;
	}

	ret = media_info_get_thumbnail_path(item, &(mdata->thumbnail_path));
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_SDATA_ERROR("media_info_foreach_media_from_db is failed, err = %s", _strerror_db(ret));
		goto ALLOC_MHANDLE_ERROR;
	}
	if (mdata->thumbnail_path == NULL)
	{
		MSG_SDATA_ERROR("thumbnail is NULL, request to DB later");
		mdata->thumbnail_path = strdup(DEFAULT_THUMBNAIL_PATH);
	}

	media_content_type_e media_type = 0;
	ret = media_info_get_media_type(item, &media_type);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_SDATA_ERROR("media_info_get_media_type is failed, err = %s", _strerror_db(ret));
		goto ALLOC_MHANDLE_ERROR;
	}

	Ivug_DB_Stroge_Type storage_type = IV_STORAGE_TYPE_INTERNAL;
	ivug_db_get_file_storage_type(media, &storage_type);

	if (media_type == MEDIA_CONTENT_TYPE_IMAGE)
	{
		{
			MSG_SDATA_MED("Local image : %s. %s", uuid_getchar(mdata->mediaID), mdata->fileurl);

			mdata->slide_type = SLIDE_TYPE_IMAGE;
			mdata->filepath = strdup(mdata->fileurl);

			Ivug_Media_Type mType;

			ivug_db_get_media_type(media, &mType);

			switch (mType)
			{
				case IV_MEDIA_TYPE_PANORAMA:
					mdata->iType = MIMAGE_TYPE_PANORAMA;
					break;

				case IV_MEDIA_TYPE_SOUNDNSHOT:
					mdata->iType = MIMAGE_TYPE_SOUNDSCENE;
					break;

				case IV_MEDIA_TYPE_ANIMATED_PHOTO:
					mdata->iType = MIMAGE_TYPE_ANIMATED;
					break;

				case IV_MEDIA_TYPE_BEST_PHOTO:
					mdata->iType = MIMAGE_TYPE_BESTSHOT;
					break;

				case IV_MEDIA_TYPE_NORMAL:
				default:
					mdata->iType = MIMAGE_TYPE_NORMAL;
					break;
			}
		}
	}
	else if (media_type == MEDIA_CONTENT_TYPE_VIDEO)
	{
		MSG_SDATA_MED("Local video : %s. %s", uuid_getchar(mdata->mediaID), mdata->fileurl);

		mdata->slide_type = SLIDE_TYPE_VIDEO;
		mdata->filepath = strdup(mdata->fileurl);
	}
	else
	{
		MSG_SDATA_ERROR("media_type is invalid %d", media_type);
		mdata->slide_type = SLIDE_TYPE_UNKNOWN;
		mdata->filepath = strdup(DEFAULT_THUMBNAIL_PATH);
	}

	if (ivug_is_web_uri(mdata->fileurl) == false)
	{
		//if (ivug_is_supported_file_type(mdata->fileurl) == false)
		if (ivug_db_is_supported_file_type(mdata->m_handle) == false)
		{
			MSG_SDATA_WARN("file path is not supported = %s", mdata->fileurl);
			mdata->state = DATA_STATE_ERROR;
		}
		else
		{
			mdata->state = DATA_STATE_READY;
		}
	}
	else
	{
		mdata->state = DATA_STATE_READY;
	}

	return mdata;

ALLOC_MHANDLE_ERROR:
	if (mdata)
	{
		ivug_free_mediadata(mdata);
	}
	return NULL;
}

Media_Data *ivug_alloc_mediadata_from_filepath(const char *filepath)
{
	IV_ASSERT(filepath != NULL);

	Media_Data * mdata = NULL;

	media_handle m_handle = ivug_db_get_file_handle(filepath);
	if (m_handle)
	{
		mdata = ivug_alloc_mediadata_from_media_handle(m_handle);
		ivug_db_destroy_file_handle(m_handle);
		if (mdata != NULL)
		{
			MSG_SDATA_HIGH("File path founded in DB.");
			return mdata;
		}
	}
	// Some file does not have media handle ex) USB host, Not inserted in DB yet

	MSG_SDATA_WARN("Cannot find in DB.");

	if (ivug_is_file_exist(filepath) == false)
	{
		MSG_SDATA_ERROR("%s is not exist at device", filepath);
		return NULL;
	}

	mdata = (Media_Data *) calloc(1, sizeof(Media_Data));
	if (mdata == NULL)
	{
		MSG_SDATA_ERROR("Cannot allocate memory");
		return NULL;
	}

	mdata->mediaID = INVALID_UUID;	//copy id WMitem

	char *mime_type = NULL;
	mime_type = ivug_fileinfo_get_mime_type(filepath);
	if (mime_type == NULL)
	{
		MSG_SDATA_WARN("file path is not vaild = %s", filepath);
		mdata->slide_type = SLIDE_TYPE_UNKNOWN;
		mdata->fileurl = NULL;
		mdata->filepath = NULL;
		return mdata;
	}

	Media_Type slide_type = SLIDE_TYPE_NONE;
	//image
	if (strncmp(mime_type, "image/", strlen("image/")) == 0)
	{
		slide_type = SLIDE_TYPE_IMAGE;
	}
	else if (strncmp(mime_type, "video/", strlen("video/")) == 0)
	{
		slide_type = SLIDE_TYPE_VIDEO;
	}
	else
	{
		slide_type = SLIDE_TYPE_NONE;
	}
	MSG_SDATA_HIGH("File=%s Mime=%s", filepath, mime_type);
	free(mime_type);		//free strdup
	if (slide_type == SLIDE_TYPE_IMAGE)
	{
		{
			mdata->slide_type = SLIDE_TYPE_IMAGE;
			mdata->fileurl = strdup(filepath);
			if (mdata->fileurl == NULL)
			{
				MSG_SDATA_ERROR("strdup return NULL");
				goto ERROR;
			}
			mdata->filepath = strdup(filepath);
			if (mdata->filepath == NULL)
			{
				MSG_SDATA_ERROR("strdup return NULL");
				free(mdata->fileurl);
				goto ERROR;
			}

			mdata->iType = MIMAGE_TYPE_NORMAL;
			mdata->thumbnail_path = strdup(DEFAULT_THUMBNAIL_PATH);
		}
	}
	else if (slide_type == SLIDE_TYPE_VIDEO)
	{
		{
			mdata->slide_type = SLIDE_TYPE_VIDEO;
			mdata->fileurl = strdup(filepath);
			if (mdata->fileurl == NULL)
			{
				MSG_SDATA_ERROR("strdup return NULL");
				goto ERROR;
			}
			mdata->filepath = strdup(filepath);
			if (mdata->filepath == NULL)
			{
				MSG_SDATA_ERROR("strdup return NULL");
				free(mdata->fileurl);
				goto ERROR;
			}
			mdata->thumbnail_path = strdup(DEFAULT_THUMBNAIL_PATH);
		}
	}
	else
	{
		MSG_SDATA_WARN("file path is not vaild = %s", filepath);
		mdata->slide_type = SLIDE_TYPE_UNKNOWN;
		mdata->fileurl = NULL;
		mdata->filepath = NULL;
	}

	if (ivug_is_web_uri(mdata->fileurl) == false)
	{
		if (ivug_is_supported_file_type(mdata->fileurl) == false)
		{
			MSG_SDATA_WARN("file path is not supported = %s", mdata->fileurl);
			mdata->state = DATA_STATE_ERROR;
		}
		else
		{
			mdata->state = DATA_STATE_READY;
		}
	}
	else
	{
		mdata->state = DATA_STATE_READY;
	}

	return mdata;
ERROR:
	if (mdata)
	{
		ivug_free_mediadata(mdata);
	}
	return NULL;
}

Media_Data *ivug_alloc_mediadata_from_url(const char *url)
{
	IV_ASSERT(url != NULL);

	Media_Data * mdata = NULL;
	mdata = (Media_Data *) calloc(1, sizeof(Media_Data));
	if (mdata == NULL)
	{
		MSG_SDATA_ERROR("Cannot allocate memory");
		return NULL;
	}

	//file url
	mdata->fileurl = strdup(url);
	if (mdata->fileurl == NULL)
	{
		MSG_SDATA_ERROR("strdup return NULL");
		free(mdata);
		return NULL;
	}

	mdata->state = DATA_STATE_READY;

	return mdata;
}

void ivug_free_mediadata(Media_Data * mdata)
{
	IV_ASSERT(mdata != NULL);

	mdata->thumbnail_callback = NULL;
	mdata->file_callback = NULL;

#ifdef USE_NEW_DB_API
	if (mdata->thumb_handle)
	{
		ivug_db_cancel_thumbnail(mdata->thumb_handle);
		mdata->thumb_handle = NULL;
	}

	if (mdata->m_handle)
	{
		media_info_destroy(mdata->m_handle);
		mdata->m_handle = NULL;
	}
#endif

	uuid_free(mdata->mediaID);

	//file path
	if (mdata->filepath)
	{
		//IVUG_DEBUG_MSG("filepath =%s", sd->filepath);
		free(mdata->filepath);
		mdata->filepath = NULL;
	}

	//file url
	if (mdata->fileurl)
	{
		MSG_SDATA_MED("Remove media data. %s", mdata->fileurl);
		free(mdata->fileurl);
		mdata->fileurl = NULL;
	}

	if (mdata->thumbnail_path)
	{
		free(mdata->thumbnail_path);
		mdata->thumbnail_path = NULL;
	}

	free(mdata);
}

bool ivug_mediadata_set_tag(Media_Data *data, const char *newtag)
{
	ivug_retv_if(!newtag, false);

	MSG_UTIL_HIGH("Add Tag : uuid(%s), tag_name=%s", uuid_getchar(data->mediaID), newtag);

	tag_handle tag = NULL;
	bool ret = ivug_db_create_tag(&tag, newtag);
	if (ret == false)
	{
		MSG_SDATA_WARN("tag already created");
		tag = ivug_db_get_tag_handle(newtag);
		if (tag == NULL)
		{
			MSG_SDATA_ERROR("ivug_db_get_tag_handle ERROR");
			return false;
		}
	}

	ret = ivug_db_set_tag(data->m_handle, tag);
	if (ret == false)
	{
		MSG_SDATA_ERROR("ivug_db_set_tag ERROR");
	}

	ivug_db_destroy_tag(tag);

	return ret;
}

bool ivug_mediadata_set_favorite(Media_Data *data, bool bFavorite)
{
	MSG_UTIL_HIGH("Add Favorite : uuid(%s), bFavorite=%d", uuid_getchar(data->mediaID), bFavorite);

	return ivug_db_set_favorite(data->m_handle, bFavorite);
}

bool ivug_mediadata_get_favorite(Media_Data *data, bool *bFavorite)
{
	bool ret = ivug_db_get_favorite(data->m_handle, bFavorite);

	MSG_UTIL_HIGH("Get Favorite : uuid(%s), bFavorite=%d", uuid_getchar(data->mediaID), *bFavorite);

	return ret;
}



bool ivug_mediadata_delete(Media_Data * mdata)
{
	IV_ASSERT(mdata != NULL);

	if (uuid_is_valid(mdata->mediaID) == true)
	{
		int ret = MEDIA_CONTENT_ERROR_NONE;

		ret = media_info_delete_from_db(mdata->mediaID);

		if (ret == MEDIA_CONTENT_ERROR_NONE)
		{
			if (mdata->filepath)
			{
				MSG_SDATA_HIGH("File removed. %s", mdata->filepath);
				if (ivug_remove_file(mdata->filepath) == false)
				{
					MSG_SDATA_ERROR("file remove error : %s", mdata->filepath);
				}
			}
			else
			{
				MSG_SDATA_ERROR("File path is NULL", mdata->filepath);
			}
		}
		else
		{
			MSG_SDATA_ERROR("media_info_delete_from_db faild=%d uuid=%s file=%s", ret, uuid_getchar(mdata->mediaID), mdata->filepath);
			return false;
		}
	}
	else
	{
		MSG_SDATA_WARN("Invalid UUID. Path=%s", mdata->filepath);

		if (mdata->filepath)
		{
			MSG_SDATA_HIGH("File removed. %s", mdata->filepath);

			if (ivug_remove_file(mdata->filepath) == false)
			{
				MSG_SDATA_ERROR("file remove error : %s", mdata->filepath);
			}
		}
		else
		{
			MSG_SDATA_ERROR("File path is NULL", mdata->filepath);
		}

	}

	return true;
}

static void _thumbnail_cb(media_handle media, const char *path, void *data)
{
	MSG_SDATA_HIGH("_thumbnail_cb, path = %s", path);

	Media_Data *mdata = (Media_Data *)data;

	mdata->thumbnail_path = strdup(path);

	ivug_db_cancel_thumbnail(mdata->thumb_handle);

	mdata->thumb_handle = NULL;

	_ivug_mediadata_call_thumbnail_callback(mdata, DATA_STATE_LOADED);
}

void ivug_mediadata_request_thumbnail(Media_Data *mdata, mdata_callback_t callback, void *data)
{

	if (callback)
	{
		mdata->thumbnail_callback = callback;
		mdata->thumb_cb_data = data;
	}

	switch (mdata->slide_type)
	{
		case SLIDE_TYPE_IMAGE:
		case SLIDE_TYPE_VIDEO:
			if (mdata->thumb_handle == NULL)
			{
				mdata->thumb_handle = ivug_db_create_thumbnail(mdata->m_handle, _thumbnail_cb, (void *)mdata);
			}
			else
			{
				_ivug_mediadata_call_thumbnail_callback(mdata, DATA_STATE_LOADED);
			}
			break;
		case SLIDE_TYPE_UNKNOWN:
			MSG_SITEM_ERROR("Unknown image. %s", ivug_get_filename(mdata->filepath));
			_ivug_mediadata_call_thumbnail_callback(mdata, DATA_STATE_ERROR);
			break;

		default:
			MSG_SITEM_ERROR("slide type invaild. Type=%d", mdata->slide_type);
			_ivug_mediadata_call_thumbnail_callback(mdata, DATA_STATE_ERROR);
			break;

	}
}

void ivug_mediadata_request_file(Media_Data *mdata, mdata_callback_t callback, void *data)
{
	if (mdata->state == DATA_STATE_LOADING)
	{
		MSG_SITEM_HIGH("data is loading");
		callback(mdata, DATA_STATE_LOADING, data);
		return;
	}

	if (callback)
	{
		mdata->file_callback = callback;
		mdata->cb_data = data;
	}

	Media_Item *cur_item = ivug_medialist_get_current_item(mdata->p_mList);
	Media_Data *cur_mdata = ivug_medialist_get_data(cur_item);

	MSG_SITEM_SEC("cur %s, mdata %s", cur_mdata->fileurl, mdata->fileurl);

	if (mdata->state == DATA_STATE_LOADED)
	{
		MSG_SITEM_HIGH("data is already loaded");
		_ivug_mediadata_call_file_callback(mdata, DATA_STATE_LOADED);
		return;
	}

	mdata->state = DATA_STATE_LOADING;

	switch (mdata->slide_type)
	{
		case SLIDE_TYPE_IMAGE:
		case SLIDE_TYPE_VIDEO:
			_ivug_mediadata_call_file_callback(mdata, DATA_STATE_LOADED);
			break;
		case SLIDE_TYPE_UNKNOWN:
			MSG_SITEM_ERROR("Unknown image. %s", ivug_get_filename(mdata->filepath));
			_ivug_mediadata_call_file_callback(mdata, DATA_STATE_ERROR);
			break;

		default:
			MSG_SITEM_ERROR("slide type invaild. Type=%d", mdata->slide_type);
			_ivug_mediadata_call_file_callback(mdata, DATA_STATE_ERROR);
			break;
	}
}

void ivug_mediadata_cancel_request_file(Media_Data *mdata)
{
	MSG_SITEM_SEC("ivug_mediadata_cancel_request_file, %s", mdata->fileurl);

	mdata->file_callback = NULL;

	switch (mdata->slide_type)
	{
		case SLIDE_TYPE_IMAGE:
		case SLIDE_TYPE_VIDEO:
			break;
		case SLIDE_TYPE_UNKNOWN:
			MSG_SITEM_ERROR("Unknown image. %s", ivug_get_filename(mdata->filepath));
			break;

		default:
			MSG_SITEM_ERROR("slide type invaild. Type=%d", mdata->slide_type);
			break;

	}

	if (mdata->state != DATA_STATE_LOADED)
	{
		mdata->state = DATA_STATE_READY;
	}
}

const char* ivug_mediadata_get_filepath(Media_Data *mdata)
{
	return mdata->filepath;
}

const char* ivug_mediadata_get_fileurl(Media_Data *mdata)
{
	return mdata->fileurl;
}

const char* ivug_mediadata_get_thumbnailpath(Media_Data *mdata)
{
	return mdata->thumbnail_path;
}

Data_State ivug_mediadata_get_file_state(Media_Data *mdata)
{
	return mdata->state;
}

void ivug_mediadata_set_file_state(Media_Data *mdata, Data_State state)
{
	mdata->state = state;
}

Data_State ivug_mediadata_get_thumbnail_state(Media_Data *mdata)
{
	return mdata->thumbnail_state;
}

