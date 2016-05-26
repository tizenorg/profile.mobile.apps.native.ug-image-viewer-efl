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

#include "ivug-data-list.h"
#include "ivug-uuid.h"

#include "ivug-debug.h"
#include "ivug-media.h"
#include "ivug-mediadata.h"
#include "ivug-medialist.h"//chandan

#include "ivug-file-info.h"

#include "ivug-db.h"

#include "ivug-dir.h"
#include "ivug-util.h"

#include "ivug-filter.h"

#include <Eina.h>

#include <media_content.h>
//#include <media_info_comm_feature.h>

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

static void _change_comma_to_dot(char *buffer)
{
	int len = strlen(buffer);

	int i;

	for (i=0; i<len; i++)
	{
		if (buffer[i] == ',')
		{
			buffer[i] = '.';
		}
	}
}

static bool
_ivug_list_media_item_cb(media_info_h item, void *user_data)
{
	IV_ASSERT(user_data != NULL);

	media_info_h media = NULL;
	media_info_clone(&media, item);

	Eina_List **list = (Eina_List **)user_data;
	*list = eina_list_append(*list, media);

	return true;
}

static void _load_list_db(const Filter_struct *filter, filter_handle media_filter,
	int start, int end, Eina_List **item_list)
{
	int ret = MEDIA_CONTENT_ERROR_NONE;
	DB_Filter *db_filter = filter->db_filter;

	ivug_db_set_filter_offset(media_filter, start, end);

	switch (filter->view_by)
	{
		case IVUG_VIEW_BY_FOLDER:
			ret = media_folder_foreach_media_from_db(db_filter->album_id,
						media_filter, _ivug_list_media_item_cb, item_list);
			break;

		case IVUG_VIEW_BY_HIDDEN_FOLDER:
			break;

		case IVUG_VIEW_BY_ALL:
			ret = media_info_foreach_media_from_db(media_filter, _ivug_list_media_item_cb, item_list);
			break;

		case IVUG_VIEW_BY_HIDDEN_ALL:
			break;

		case IVUG_VIEW_BY_FAVORITES:
			ret = media_info_foreach_media_from_db(media_filter, _ivug_list_media_item_cb, item_list);
			break;

		case IVUG_VIEW_BY_TAG:
			ret = media_tag_foreach_media_from_db(db_filter->tag_id, media_filter,
				_ivug_list_media_item_cb, item_list);
			break;

		case IVUG_VIEW_BY_PLACES:
			ret = media_info_foreach_media_from_db(media_filter, _ivug_list_media_item_cb, item_list);
			break;

		case IVUG_VIEW_BY_TIMELINE:
			ret = media_info_foreach_media_from_db(media_filter, _ivug_list_media_item_cb, item_list);
			break;
		default:
			MSG_SDATA_FATAL("Invalid View By=%d", filter->view_by);
			return;
	}

	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_SDATA_ERROR("[%s] Error=%d VieBy=%d", __func__, ret, filter->view_by);
	}

}

Eina_List *
ivug_list_load_DB_items(const Filter_struct *filter, int stp, int endp)
{
	IV_ASSERT(filter != NULL);

	DB_Filter *db_filter = filter->db_filter;
	char *string = NULL;

	if (filter->view_by == IVUG_VIEW_BY_FILE)
	{
		Eina_List *list = NULL;
		Media_Data *mData = ivug_alloc_mediadata_from_filepath(db_filter->file_path);

		if (mData == NULL)
		{
			MSG_SDATA_ERROR("mData is NULL");
			return NULL;
		}

		list = eina_list_append(list, mData);
		return list;
	}

	filter_handle media_filter = NULL;
	ivug_db_create_filter(&media_filter);

	if (filter->view_by == IVUG_VIEW_BY_PLACES)
	{
		string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);

		if (string != NULL) {
			snprintf(string, IVUG_MAX_CONDITION_LEN,
				"(MEDIA_LONGITUDE >= %f AND MEDIA_LONGITUDE <= %f AND MEDIA_LATITUDE >= %f AND MEDIA_LATITUDE <= %f)",
				db_filter->place.min_longitude, db_filter->place.max_longitude,
				db_filter->place.min_latitude, db_filter->place.max_latitude);

			_change_comma_to_dot(string);
		}
	}
	else if (filter->view_by == IVUG_VIEW_BY_TIMELINE)
	{
		string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);
		if (string != NULL) {
			snprintf(string, IVUG_MAX_CONDITION_LEN,
				"(MEDIA_TIMELINE BETWEEN %ld AND %ld)",
				db_filter->time.start, db_filter->time.end);
		}
	}
	ivug_db_set_filter(media_filter, filter->view_by, filter->media_type, string);

	Eina_List *item_list = NULL;

	_load_list_db(filter, media_filter, stp, endp, &item_list);

	if (filter->sort_type == IVUG_MEDIA_ASC_BY_DATE) {
		if(item_list) {
			item_list = eina_list_reverse(item_list);
		}
	}

	ivug_db_destroy_filter(media_filter);

// Creating media_list.
	Eina_List *slide_list = NULL;

	Eina_List *item;
	media_info_h mitem = NULL;

	Media_Data *mdata = NULL;

	int i = 0;

	EINA_LIST_FOREACH(item_list, item, mitem)
	{
		mdata = ivug_alloc_mediadata_from_media_handle(mitem);
		if (mdata == NULL)
		{
			MSG_SDATA_ERROR("mdata create error!");
			continue;
		}
		mdata->index = i + stp;		// stp~

		i++;

		IV_ASSERT(mdata != NULL);

		MSG_SDATA_LOW("Add Mdata. Mdata=0x%08x %s", mdata, mdata->filepath);
		slide_list = eina_list_append(slide_list, mdata);
	}

	EINA_LIST_FREE(item_list, mitem)
	{
		media_info_destroy(mitem);
	}

	MSG_SDATA_HIGH("Item header=0x%08x Item loaded(%d~%d)", slide_list, stp, endp);

	return slide_list;
}

Eina_List *
ivug_list_load_DB_items_list(const Filter_struct *filter, Eina_List *list)
{
	IV_ASSERT(filter != NULL);

	DB_Filter *db_filter = filter->db_filter;
	char *string = NULL;

	filter_handle media_filter = NULL;
	ivug_db_create_filter(&media_filter);

	if (filter->view_by == IVUG_VIEW_BY_PLACES)
	{
		string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);

		if (string != NULL) {
			snprintf(string, IVUG_MAX_CONDITION_LEN,
				"(MEDIA_LONGITUDE >= %f AND MEDIA_LONGITUDE <= %f AND MEDIA_LATITUDE >= %f AND MEDIA_LATITUDE <= %f)",
				db_filter->place.min_longitude, db_filter->place.max_longitude,
				db_filter->place.min_latitude, db_filter->place.max_latitude);
			_change_comma_to_dot(string);
		}
	}
	else if (filter->view_by == IVUG_VIEW_BY_TIMELINE)
	{
		string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);
		if (string != NULL) {
			snprintf(string, IVUG_MAX_CONDITION_LEN,
				"(MEDIA_TIMELINE BETWEEN %ld AND %ld)",
				db_filter->time.start, db_filter->time.end);
		}
	}

	ivug_db_set_filter(media_filter, filter->view_by, filter->media_type, string);

	Eina_List *l = NULL;
	void *data = NULL;
	int index = -1;
	Eina_List *item_list = NULL;
	int start = -1;
	int end = -1;

	EINA_LIST_FOREACH(list, l, data)
	{
		index = (int )data;
		if (start == -1)
		{
			start = index;
			end = start;
			continue;
		}
		else if (index == end+1)
		{
			end = index;
			continue;
		}
		else
		{
			_load_list_db(filter, media_filter, start, end, &item_list);

			start = index;
			end = index;
		}
	}
	_load_list_db(filter, media_filter, start, end, &item_list);	//load remain

	ivug_db_destroy_filter(media_filter);

	MSG_SDATA_HIGH("DB Item loaded %d items", eina_list_count(item_list));

// Creating media_list.
	Eina_List *slide_list = NULL;

	Eina_List *item;
	media_info_h mitem = NULL;

	Media_Data *mdata = NULL;

	int i = 0;

	EINA_LIST_FOREACH(item_list, item, mitem)
	{
		mdata = ivug_alloc_mediadata_from_media_handle(mitem);
		if (mdata == NULL)
		{
			MSG_SDATA_ERROR("mdata create error!");
			continue;
		}
		mdata->index = i;

		i++;

		IV_ASSERT(mdata != NULL);

		MSG_SDATA_LOW("Add Mdata. Mdata=0x%08x %s", mdata, mdata->filepath);
		slide_list = eina_list_append(slide_list, mdata);
	}

	EINA_LIST_FREE(item_list, mitem)
	{
		media_info_destroy(mitem);
	}

	MSG_SDATA_HIGH("Item header=0x%08x Item loaded %d items", slide_list, eina_list_count(slide_list));

	return slide_list;
}


int ivug_list_get_item_cnt(const Filter_struct *filter)
{
	filter_h media_filter = NULL;

	int ret = MEDIA_CONTENT_ERROR_NONE;

	int count = 0;

	char *string = NULL;

	DB_Filter *db_filter = filter->db_filter;

	ivug_db_create_filter((filter_handle*)&media_filter);
	if (filter->view_by == IVUG_VIEW_BY_PLACES)
	{
		string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);

		if (string != NULL) {
			snprintf(string, IVUG_MAX_CONDITION_LEN,
				"(MEDIA_LONGITUDE >= %f AND MEDIA_LONGITUDE <= %f AND MEDIA_LATITUDE >= %f AND MEDIA_LATITUDE <= %f)",
				db_filter->place.min_longitude, db_filter->place.max_longitude,
				db_filter->place.min_latitude, db_filter->place.max_latitude);
			_change_comma_to_dot(string);
		}
	}
	else if (filter->view_by == IVUG_VIEW_BY_TIMELINE)
	{
		string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);
		if (string != NULL) {
			snprintf(string, IVUG_MAX_CONDITION_LEN,
				"(MEDIA_TIMELINE BETWEEN %ld AND %ld)",
				db_filter->time.start, db_filter->time.end);
		}
	}
	ivug_db_set_filter(media_filter, filter->view_by, filter->media_type, string);

	switch (filter->view_by)
	{
		case IVUG_VIEW_BY_ALL:
			ret = media_info_get_media_count_from_db(media_filter, &count);
			if (ret != MEDIA_CONTENT_ERROR_NONE)
			{
				MSG_SDATA_ERROR("media_info_get_media_count_from_db is failed, err = %s", _strerror_db(ret));
				goto GET_COUNT_ERROR;
			}
			MSG_SDATA_HIGH("IVUG_VIEW_BY_ALL. MediaType=%d Count=%d", filter->media_type, count);
			break;
		case IVUG_VIEW_BY_HIDDEN_ALL:
			break;
		case IVUG_VIEW_BY_TAG:
			if (db_filter->tag_id <= 0)
			{
				MSG_SDATA_ERROR("tag id is invalid");
				goto GET_COUNT_ERROR;
			}
			ret = media_tag_get_media_count_from_db(db_filter->tag_id, media_filter, &count);
			if (ret != MEDIA_CONTENT_ERROR_NONE)
			{
				MSG_SDATA_ERROR("media_tag_get_tag_count_from_db is failed, err = %s", _strerror_db(ret));
				goto GET_COUNT_ERROR;
			}
			MSG_SDATA_HIGH("IVUG_VIEW_BY_TAG. Count=%d", count);
			break;
		case IVUG_VIEW_BY_PLACES:
			ret = media_info_get_media_count_from_db(media_filter, &count);
			if (ret != MEDIA_CONTENT_ERROR_NONE)
			{
				MSG_SDATA_ERROR("media_info_get_media_count_from_db is failed, err = %s", _strerror_db(ret));
				goto GET_COUNT_ERROR;
			}
			MSG_SDATA_HIGH("IVUG_VIEW_BY_PLACES. Count=%d", count);
			break;
		case IVUG_VIEW_BY_TIMELINE:
			ret = media_info_get_media_count_from_db(media_filter, &count);
			if (ret != MEDIA_CONTENT_ERROR_NONE)
			{
				MSG_SDATA_ERROR("media_info_get_media_count_from_db is failed, err = %s", _strerror_db(ret));
				goto GET_COUNT_ERROR;
			}
			MSG_SDATA_HIGH("IVUG_VIEW_BY_TIMELINE. Count=%d", count);
			break;
		case IVUG_VIEW_BY_FOLDER:
			if (db_filter->album_id == NULL)
			{
				MSG_SDATA_ERROR("album_id is NULL");
				goto GET_COUNT_ERROR;
			}
			ret = media_folder_get_media_count_from_db(db_filter->album_id, media_filter, &count);
			if (ret != MEDIA_CONTENT_ERROR_NONE)
			{
				MSG_SDATA_ERROR("media_folder_get_media_count_from_db is failed, err = %s", _strerror_db(ret));
				goto GET_COUNT_ERROR;
			}
			MSG_SDATA_HIGH("IVUG_VIEW_BY_FOLDER. Count=%d", count);
			break;
		case IVUG_VIEW_BY_HIDDEN_FOLDER:
			if (db_filter->album_id == NULL)
			{
				MSG_SDATA_ERROR("album_id is NULL");
				goto GET_COUNT_ERROR;
			}
			break;
		case IVUG_VIEW_BY_FAVORITES:
			ret = media_info_get_media_count_from_db(media_filter, &count);
			if (ret != MEDIA_CONTENT_ERROR_NONE)
			{
				MSG_SDATA_ERROR("media_info_get_media_count_from_db is failed, err = %s", _strerror_db(ret));
				goto GET_COUNT_ERROR;
			}
			MSG_SDATA_HIGH("IVUG_VIEW_BY_FAVORITES. Count=%d", count);
			break;
		case IVUG_VIEW_BY_FILE:
			count = 1;	// item count is always 1 when by file
			break;
		default:
			MSG_SDATA_ERROR("Unhandled view_by : %d", filter->view_by);
			goto GET_COUNT_ERROR;
			break;
	}

	MSG_SDATA_HIGH("ivug_db_get_count success, count = %d", count);

	ivug_db_destroy_filter(media_filter);
	return count;

GET_COUNT_ERROR:
	MSG_SDATA_ERROR("ivug_db_get_count FAILED");

	ivug_db_destroy_filter(media_filter);
	return -1;
}

int ivug_list_get_burst_item_cnt(const Filter_struct *filter, const char * burst_id)
{
	filter_h media_filter = NULL;

	int ret = MEDIA_CONTENT_ERROR_NONE;

	int count = 0;

	char *string = NULL;

	ivug_db_create_filter((filter_handle*)&media_filter);

	string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);
	
	if (string != NULL) {
		snprintf(string, IVUG_MAX_CONDITION_LEN, "(%s=%s)", MEDIA_BURST_ID, burst_id);
		ivug_db_set_image_time_asc_filter(media_filter, string);
	}

	ret = media_info_get_media_count_from_db(media_filter, &count);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_SDATA_ERROR("media_info_get_media_count_from_db is failed, err = %s", _strerror_db(ret));
		goto GET_COUNT_ERROR;
	}

	MSG_SDATA_HIGH("ivug_db_get_count success, count = %d", count);

	ivug_db_destroy_filter(media_filter);
	return count;

GET_COUNT_ERROR:
	MSG_SDATA_ERROR("ivug_db_get_count FAILED");

	ivug_db_destroy_filter(media_filter);
	return -1;
}


void ivug_list_delete_items(Eina_List *items)
{
	Media_Data* mdata;

	if (items == NULL)
	{
		MSG_SDATA_HIGH("Item list is NULL.");
		return;
	}

	MSG_SDATA_HIGH("Removing items. Count=%d", eina_list_count(items));

	EINA_LIST_FREE(items, mdata)
	{
		ivug_free_mediadata(mdata);
	}
}

static void enum_dir(const char *fname, void *user_data)
{
	IV_ASSERT(user_data != NULL);

	char *mime_type = NULL;
	mime_type = ivug_fileinfo_get_mime_type(fname);
	if (mime_type == NULL)
	{
		MSG_SDATA_WARN("file path is not vaild = %s", fname);
		return;
	}

	Media_Type slide_type = SLIDE_TYPE_NONE;
	//image
	if (strncmp(mime_type, "image/jpeg", strlen("image/jpeg")) == 0
		|| strncmp(mime_type, "image/bmp", strlen("image/bmp")) == 0
		|| strncmp(mime_type, "image/png", strlen("image/png")) == 0
		|| strncmp(mime_type, "image/gif", strlen("image/gif")) == 0
		|| strncmp(mime_type, "image/vnd.wap.wbmp", strlen("image/vnd.wap.wbmp")) == 0)
	{
		slide_type = SLIDE_TYPE_IMAGE;
	}
	else if (strncmp(mime_type, "video/", strlen("video/")) == 0)
	{
		slide_type = SLIDE_TYPE_VIDEO;
	}
	else
	{
		MSG_SDATA_WARN("not supported file type = %s", fname);
		free(mime_type);
		return;
	}

	free(mime_type);

	Eina_List **list = (Eina_List **)user_data;

// Alloc new mdata
	Media_Data *mdata = NULL;

	mdata = (Media_Data *)calloc(1, sizeof(Media_Data));

	if (mdata != NULL) {
		// Thumbnail path format.. ".thumbs/xxxx_thm.ext"
		// File path format.. "xxxx_bestpic.ext"
		mdata->fileurl = strdup(fname);
		mdata->filepath = strdup(fname);
		mdata->slide_type = slide_type;

		// Get thumblist.
		// User free return value.
		char *dir = ivug_get_directory(fname);
		// User should not free return value.
		const char *file = ivug_get_filename(fname);
		
		if (dir != NULL) {
			char *thumbpath = (char *)malloc(strlen(dir) + strlen(file) + strlen("/.thumbs/")+ 1);
			int size_x = (strlen(dir) + strlen(file) + strlen("/.thumbs/")+ 1);
			if (thumbpath == NULL)
			{
				MSG_SDATA_WARN("malloc ERROR");
				free(mdata->filepath);
				free(mdata->fileurl);
				free(mdata);
				free(dir);
				return;
			}

			snprintf(thumbpath, size_x, "%s/.thumbs/%s", dir, file);

			mdata->thumbnail_path = thumbpath; // thumbnail image file path.
			free(dir);
		}

		*list = eina_list_append(*list, (void *)mdata);
	}
}

int ivug_list_get_dir_cnt(const char *basedir)
{
	int count = 0;

	count = GetFilesCountInDir(basedir);

	if (count < 0)
	{
		MSG_SDATA_ERROR("Cannot get item count in dir : %s", basedir);
		return 0;
	}

	MSG_SDATA_HIGH("IVUG_VIEW_BY_DIRECTORY. Count=%d", count);

	return count;
}


Eina_List *
ivug_list_load_dir_items(const char *basedir)
{
	Eina_List *list = NULL;

	bool result = false;

	result = EnumFilesInDir(basedir, enum_dir, &list);

	if (result == false)
	{
		MSG_SDATA_ERROR("Cannit get file list in %s", basedir);
		return NULL;
	}

	return list;
}


Eina_List *ivug_list_append_item(Eina_List *list, const char *filepath)
{
	Media_Data *mData = ivug_alloc_mediadata_from_filepath(filepath);

	list = eina_list_append(list, mData);
	return list;
}

Eina_List *ivug_list_prepend_item(Eina_List *list, const char *filepath)
{
	Media_Data *mData = ivug_alloc_mediadata_from_filepath(filepath);

	list = eina_list_prepend(list, mData);
	return list;
}

Eina_List *
ivug_list_load_burst_items(const Filter_struct *filter, const char *burst_id, int stp, int endp)
{
	IV_ASSERT(filter != NULL);

	char *string = NULL;

	filter_handle media_filter = NULL;
	ivug_db_create_filter(&media_filter);

	string = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);

	if (string != NULL) {
		snprintf(string, IVUG_MAX_CONDITION_LEN, "(%s=%s)", MEDIA_BURST_ID, burst_id);
		ivug_db_set_image_time_asc_filter(media_filter, string);
	}

	Eina_List *item_list = NULL;

	_load_list_db(filter, media_filter, stp, endp, &item_list);

	ivug_db_destroy_filter(media_filter);

// Creating media_list.
	Eina_List *slide_list = NULL;

	Eina_List *item;
	media_info_h mitem = NULL;

	Media_Data *mdata = NULL;

	int i = 0;

	EINA_LIST_FOREACH(item_list, item, mitem)
	{
		mdata = ivug_alloc_mediadata_from_media_handle(mitem);
		if (mdata == NULL)
		{
			MSG_SDATA_ERROR("mdata create error!");
			continue;
		}
		mdata->index = i + stp;		// stp~

		i++;

		IV_ASSERT(mdata != NULL);

		MSG_SDATA_LOW("Add Mdata. Mdata=0x%08x %s", mdata, mdata->filepath);
		slide_list = eina_list_append(slide_list, mdata);
	}

	EINA_LIST_FREE(item_list, mitem)
	{
		media_info_destroy(mitem);
	}

	MSG_SDATA_HIGH("Item header=0x%08x Item loaded(%d~%d)", slide_list, stp, endp);

	return slide_list;
}

Eina_List *
ivug_list_load_file_list(const Filter_struct *filter, Eina_List *list)
{
	IV_ASSERT(filter != NULL);

// Creating media_list.
	Eina_List *slide_list = NULL;
	Eina_List *item;
	Media_Data *mdata = NULL;
	char *filepath = NULL;

	int i = 0;

	EINA_LIST_FOREACH(list, item, filepath)
	{
		mdata = ivug_alloc_mediadata_from_filepath(filepath);
		if (mdata == NULL)
		{
			MSG_SDATA_ERROR("mdata create error!");
			continue;
		}
		mdata->index = i;

		i++;

		IV_ASSERT(mdata != NULL);

		MSG_SDATA_LOW("Add Mdata. Mdata=0x%08x %s", mdata, mdata->filepath);
		slide_list = eina_list_append(slide_list, mdata);
	}

	MSG_SDATA_HIGH("Item header=0x%08x Item loaded %d items", slide_list, eina_list_count(slide_list));

	return slide_list;
}

