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
#include "ivug-debug.h"
#include "ivug-util.h"
#include "ivug-db.h"

#include <Elementary.h>
#include <media_content.h>
//#include <media_info_comm_feature.h>

#include <time.h>		// localtime_r

#define DB_QUERY_STORAGE_ALL "(MEDIA_STORAGE_TYPE=0 OR MEDIA_STORAGE_TYPE=1 OR MEDIA_STORAGE_TYPE=101 OR MEDIA_STORAGE_TYPE=121)"
#define LOG_LVL DBG_MSG_LVL_HIGH
#define LOG_CAT "IV-DB"

#define DB_KEY	(0x12341234)

typedef struct _Ivug_DB
{
	ivug_db_cb callback;
	media_handle *m_handle;
	tag_handle *t_handle;
	void *data;
}Ivug_DB;

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
			snprintf(buf, 40, "Unknwon Error(%d)", error);
			return buf;
		}

	}
}

static void _ivug_thumb_cb(media_content_error_e error,
				       const char *path, void *data)
{
	Ivug_DB_h *db_h = (Ivug_DB_h *)data;

	MSG_HIGH("_ivug_thumb_cb, path =%s", path);

	if (db_h->callback == NULL || db_h->key != DB_KEY)
	{
		MSG_WARN("handle was freed");
		return;
	}

	if (path == NULL)
	{
		MSG_ERROR("thumbnail path is NULL");
		db_h->callback(db_h->m_handle, NULL, db_h->data);
		return;
	}

	db_h->callback(db_h->m_handle, path, db_h->data);
}

bool ivug_db_create(void)
{
	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_content_connect();
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_content_connect is failed, err:%s", _strerror_db(ret));
		return false;
	}

	MSG_HIGH("ivug_db_create success");
	return true;
}

bool ivug_db_destroy(void)
{
	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_content_disconnect();
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_content_disconnect is failed, err:%s", _strerror_db(ret));
		return false;
	}

	MSG_HIGH("ivug_db_destroy success");
	return true;
}

bool ivug_db_rename(media_handle m_handle, const char *dest)
{
	media_info_h minfo = (media_info_h)m_handle;
	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_info_move_to_db(minfo, dest);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_move_to_db is failed, err = %s dest=%s", _strerror_db(ret), dest);
		return false;
	}

	return true;
}

bool ivug_db_destroy_file_handle(media_handle m_handle)
{
	media_info_h minfo = (media_info_h)m_handle;
	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_info_destroy(minfo);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_destroy is failed, err = %s", _strerror_db(ret));
		return false;
	}

	return true;
}

bool ivug_db_destroy_folder_handle(media_handle m_handle)
{
	media_folder_h minfo = (media_folder_h)m_handle;
	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_folder_destroy(minfo);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_folder_destroy is failed, err = %s", _strerror_db(ret));
		return false;
	}

	return true;
}

bool ivug_db_create_filter(filter_handle *filter)
{
	filter_h *media_filter = (filter_h *)filter;
	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_filter_create(media_filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_filter_create is failed, err = %s", _strerror_db(ret));
		return false;
	}
	MSG_MED("ivug_db_create_filter success");
	return true;
}

bool ivug_db_destroy_filter(filter_handle filter)
{
	filter_h media_filter = (filter_h)filter;

	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_filter_destroy(media_filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_filter_destroy is failed, err = %s", _strerror_db(ret));
		return false;
	}
	MSG_MED("ivug_db_destroy_filter success");
	return true;
}

bool ivug_db_set_filter_condition(filter_handle media_filter, const char *condition)
{
	IV_ASSERT(condition != NULL);

	MSG_SEC("Set DB Filter : %s", condition);
	int ret = media_filter_set_condition(media_filter, condition, MEDIA_CONTENT_COLLATE_DEFAULT);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_filter_set_condition is failed, err = %s", _strerror_db(ret));
		return false;
	}
	return true;
}

bool ivug_db_set_filter(filter_handle filter, ivug_view_by view_by,	ivug_media_type media_type, char *condition)
{
	filter_h media_filter = (filter_h)filter;

	int ret = MEDIA_CONTENT_ERROR_NONE;

	if (condition == NULL)
	{
		condition = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);
	}
	else
	{
		strncat(condition, " AND ", sizeof(" AND "));
	}
	
	IV_ASSERT(condition != NULL);
	/*MEDIA_TYPE 0-image, 1-video, 2-sound, 3-music*/
	switch (media_type)
	{
		case IVUG_MEDIA_TYPE_IMAGE:
			strncat(condition, "(MEDIA_TYPE=0)", sizeof("(MEDIA_TYPE=0)"));
			break;
		case IVUG_MEDIA_TYPE_VIDEO:
			strncat(condition,"(MEDIA_TYPE=1)", sizeof("(MEDIA_TYPE=1)"));
			break;
		case IVUG_MEDIA_TYPE_ALL:
			strncat(condition, "(MEDIA_TYPE=0 OR MEDIA_TYPE=1)", sizeof("(MEDIA_TYPE=0 OR MEDIA_TYPE=1)"));
			break;
		default:
			MSG_FATAL("Invalid media type : %d", media_type);
			break;
	}

	if (view_by == IVUG_VIEW_BY_FAVORITES)
	{
		strncat(condition, " AND MEDIA_FAVOURITE>0", sizeof(" AND MEDIA_FAVOURITE>0"));
	}

	if (condition)
	{
		if (ivug_db_set_filter_condition(media_filter, condition) == false)
		{
			goto SET_FILTER_ERROR;
		}
	}

	if (view_by == IVUG_VIEW_BY_TAG)
	{
		ret = media_filter_set_order(media_filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_DISPLAY_NAME, MEDIA_CONTENT_COLLATE_NOCASE);
	}
	else
	{
		ret = media_filter_set_order(media_filter, MEDIA_CONTENT_ORDER_DESC, "MEDIA_TIMELINE, MEDIA_DISPLAY_NAME", MEDIA_CONTENT_COLLATE_NOCASE);
	}
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_filter_set_order is failed, err = %s", _strerror_db(ret));
		goto SET_FILTER_ERROR;
	}

	if (condition)
		free(condition);

	MSG_MED("ivug_db_set_filter success, view_by:%d, media_type:%d", view_by, media_type);
	return true;

SET_FILTER_ERROR:
	if (condition)
		free(condition);
	return false;
}

bool ivug_db_set_image_time_asc_filter(filter_handle filter, char *condition)
{
	filter_h media_filter = (filter_h)filter;

	int ret = MEDIA_CONTENT_ERROR_NONE;

	if (condition == NULL)
	{
		condition = calloc(1, sizeof(char)*IVUG_MAX_CONDITION_LEN);
	}
	else
	{
		strncat(condition, " AND ", sizeof(" AND "));
	}

	IV_ASSERT(condition != NULL);
	strncat(condition, "(MEDIA_TYPE=0)", sizeof("(MEDIA_TYPE=0)"));

	if (ivug_db_set_filter_condition(media_filter, condition) == false)
	{
		goto SET_FILTER_ERROR;
	}

	ret = media_filter_set_order(media_filter, MEDIA_CONTENT_ORDER_ASC, "MEDIA_TIMELINE, MEDIA_DISPLAY_NAME", MEDIA_CONTENT_COLLATE_NOCASE);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_filter_set_order is failed, err = %s", _strerror_db(ret));
		goto SET_FILTER_ERROR;
	}

	free(condition);

	MSG_MED("ivug_db_set_filter success");
	return true;

SET_FILTER_ERROR:
	free(condition);
	return false;
}

bool ivug_db_set_filter_offset(filter_handle filter, int stp, int endp)
{
	filter_h media_filter = (filter_h)filter;

	int ret = MEDIA_CONTENT_ERROR_NONE;

	ret = media_filter_set_offset(media_filter, stp, endp-stp+1);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_filter_set_offset is failed, err = %s", _strerror_db(ret));
		return false;
	}

	return true;
}

media_handle ivug_db_insert_file_to_DB(const char* filepath)
{
	IV_ASSERT(filepath != NULL);

	int ret = MEDIA_CONTENT_ERROR_NONE;

	if (ivug_is_web_uri(filepath) == true)
	{
		return NULL;
	}

	media_handle m_handle = NULL;

	ret = media_info_insert_to_db(filepath, (media_info_h *)&m_handle);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_insert_to_db is failed, err = %s", _strerror_db(ret));
		return NULL;
	}

	return m_handle;
}

bool _media_item_cb(media_info_h item, void *user_data)
{
	media_handle *m_handle = (media_handle *)user_data;

	media_info_clone((media_info_h *)m_handle, item);

	return false;	//only 1 item
}

media_handle ivug_db_get_file_handle(const char* filepath)
{
	IV_ASSERT(filepath != NULL);

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_handle media_filter = NULL;

	if (ivug_is_web_uri(filepath) == true) {
		return NULL;
	}

	char buf[IVUG_MAX_FILE_PATH_LEN+256] = {0,};
	snprintf(buf, sizeof(buf), "((MEDIA_PATH=\"%s\") AND "DB_QUERY_STORAGE_ALL")", filepath);

	media_handle m_handle = NULL;

	ivug_db_create_filter(&media_filter);
	ivug_db_set_filter_condition(media_filter, buf);
	ret = media_info_foreach_media_from_db(media_filter, _media_item_cb, &m_handle);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		MSG_ERROR("media_info_foreach_media_from_db is failed, err = %s", _strerror_db(ret));
		goto GET_ITEM_ERROR;
	}
	ivug_db_destroy_filter(media_filter);

	return m_handle;

GET_ITEM_ERROR:
	if (media_filter)
		ivug_db_destroy_filter(media_filter);

	if (m_handle)
		ivug_db_destroy_file_handle(m_handle);

	return NULL;
}

media_handle ivug_db_get_file_handle_from_media_id(UUID media_id)
{
	IV_ASSERT(media_id != NULL);

	media_handle m_handle = NULL;

	int ret = media_info_get_media_from_db(media_id, (media_info_h*)&m_handle);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_media_from_db is failed, err = %s", _strerror_db(ret));
		return NULL;
	}

	return m_handle;
}

static bool _media_folder_item_cb(media_folder_h item, void *user_data)
{
	media_handle *t_handle = (media_handle *)user_data;

	media_folder_clone((media_folder_h *)t_handle, item);

	return false;	//only 1 item
}

media_handle ivug_db_get_folder_handle(const char* folderpath)
{
	IV_ASSERT(folderpath != NULL);

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_handle media_filter = NULL;

	char buf[IVUG_MAX_FILE_PATH_LEN+256] = {0,};
	snprintf(buf, sizeof(buf), "((FOLDER_PATH=\"%s\") AND "DB_QUERY_STORAGE_ALL")", folderpath);

	media_handle m_handle = NULL;

	ivug_db_create_filter(&media_filter);
	ivug_db_set_filter_condition(media_filter, buf);
	ret = media_folder_foreach_folder_from_db(media_filter, _media_folder_item_cb, &m_handle);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		MSG_ERROR("media_folder_foreach_folder_from_db is failed, err = %s", _strerror_db(ret));
		goto GET_ITEM_ERROR;
	}
	ivug_db_destroy_filter(media_filter);

	return m_handle;

GET_ITEM_ERROR:
	if (media_filter)
		ivug_db_destroy_filter(media_filter);

	return NULL;
}

static bool _media_folder_list_cb(media_folder_h item, void *user_data)
{
	Ivug_DB *iv_db = (Ivug_DB *)user_data;

	if (iv_db->callback)
	{
		iv_db->callback((media_handle)item, iv_db->data);
	}

	return true;
}

bool ivug_db_get_all_folder_list(ivug_db_cb callback, void *data)
{
	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_handle media_filter = NULL;

	Ivug_DB *iv_db = calloc(1, sizeof(Ivug_DB));
	IV_ASSERT(iv_db != NULL);
	iv_db->callback = callback;
	iv_db->data = data;

	ivug_db_create_filter(&media_filter);
	ret = media_folder_foreach_folder_from_db(media_filter, _media_folder_list_cb, iv_db);

	free(iv_db);

	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_folder_foreach_folder_from_db is failed, err = %s", _strerror_db(ret));
		goto GET_ITEM_ERROR;
	}
	ivug_db_destroy_filter(media_filter);

	return true;

GET_ITEM_ERROR:
	if (media_filter)
		ivug_db_destroy_filter(media_filter);

	return false;
}

char *ivug_db_get_file_path(media_handle media)
{
	media_info_h file_item = (media_info_h)media;
	char *name = NULL;

	media_info_get_file_path(file_item, &name);

	return name;
}

UUID ivug_db_get_file_id(media_handle media)
{
	media_info_h file_item = (media_info_h)media;
	UUID id = NULL;

	media_info_get_media_id(file_item, (char **)&id);

	return id;
}

char *ivug_db_get_thumbnail_path(media_handle media)
{
	media_info_h file_item = (media_info_h)media;
	char *name = NULL;

	media_info_get_thumbnail_path(file_item, &name);

	return name;
}

char *ivug_db_get_folder_name(media_handle media)
{
	media_folder_h folder_item = (media_folder_h)media;
	char *name = NULL;

	media_folder_get_name(folder_item, &name);

	return name;
}

UUID ivug_db_get_folder_id(media_handle media)
{
	media_folder_h folder_item = (media_folder_h)media;
	UUID id = NULL;

	media_folder_get_folder_id(folder_item, (char **)&id);

	return id;
}

char *ivug_db_get_folder_path(media_handle media)
{
	media_folder_h folder_item = (media_folder_h)media;
	char *path = NULL;

	media_folder_get_path(folder_item, &path);

	return path;
}

bool ivug_db_get_folder_storage_type(media_handle media, Ivug_DB_Stroge_Type *type)
{
//	media_folder_h folder_item = (media_folder_h)media;
//
//	int ret = MEDIA_CONTENT_ERROR_NONE;
//	media_content_storage_ex_e storage_type = MEDIA_CONTENT_STORAGE_INTERNAL_EX;
//
//	ret = media_folder_get_storage_type(folder_item, &storage_type);
//	if (ret != MEDIA_CONTENT_ERROR_NONE)
//	{
//		MSG_ERROR("media_folder_get_storage_type is failed, err = %s", _strerror_db(ret));
//		return false;
//	}
//
//	switch (storage_type)
//	{
//		case MEDIA_CONTENT_STORAGE_INTERNAL_EX:
//			*type = IV_STORAGE_TYPE_INTERNAL;
//			MSG_HIGH("storage type is INTERNAL");
//			break;
//		case MEDIA_CONTENT_STORAGE_EXTERNAL_EX:
//			*type = IV_STORAGE_TYPE_EXTERNAL;
//			MSG_HIGH("storage type is EXTERNAL");
//			break;
//		case MEDIA_CONTENT_STORAGE_DROPBOX_EX:
//			*type = IV_STORAGE_TYPE_CLOUD;
//			MSG_HIGH("storage type is CLOUD");
//			break;
//		case 121: //MEDIA_CONTENTC_STORAGE_FACEBOOK
//			*type = IV_STORAGE_TYPE_WEB;
//			MSG_HIGH("storage type is WEB");
//			break;
//		default:
//			MSG_ERROR("storage type is invalid %d", storage_type);
//			break;
//	}

	return false;
}

bool ivug_db_get_file_storage_type(media_handle media, Ivug_DB_Stroge_Type *type)
{
//	media_info_h file_item = (media_info_h)media;
//
//	int ret = MEDIA_CONTENT_ERROR_NONE;
//	media_content_storage_ex_e storage_type = MEDIA_CONTENT_STORAGE_INTERNAL_EX;
//
//	ret = media_info_get_storage_type(file_item, &storage_type);
//	if (ret != MEDIA_CONTENT_ERROR_NONE)
//	{
//		MSG_ERROR("media_info_get_storage_type is failed, err = %s", _strerror_db(ret));
//		return false;
//	}
//
//	switch (storage_type)
//	{
//		case MEDIA_CONTENT_STORAGE_INTERNAL_EX:
//			*type = IV_STORAGE_TYPE_INTERNAL;
//			MSG_MED("storage type is INTERNAL");
//			break;
//		case MEDIA_CONTENT_STORAGE_EXTERNAL_EX:
//			*type = IV_STORAGE_TYPE_EXTERNAL;
//			MSG_MED("storage type is EXTERNAL");
//			break;
//		case MEDIA_CONTENT_STORAGE_DROPBOX_EX:
//			*type = IV_STORAGE_TYPE_CLOUD;
//			MSG_HIGH("storage type is CLOUD");
//			break;
//		case 121: //MEDIA_CONTENTC_STORAGE_FACEBOOK
//			*type = IV_STORAGE_TYPE_WEB;
//			MSG_HIGH("storage type is WEB");
//			break;
//		default:
//			MSG_ERROR("storage type is invalid %d", storage_type);
//			break;
//	}

	return false;
}

static bool _media_tag_list_cb(media_tag_h item, void *user_data)
{
	Ivug_DB *iv_db = (Ivug_DB *)user_data;

	if (iv_db->callback)
	{
		iv_db->callback((media_handle)item, iv_db->data);
	}

	return true;
}

bool ivug_db_get_all_tag_list(ivug_db_cb callback, void *data)
{
	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_handle media_filter = NULL;

	Ivug_DB *iv_db = calloc(1, sizeof(Ivug_DB));
	
	if (iv_db == NULL) {
		return false;
	}

	iv_db->callback = callback;
	iv_db->data = data;

	ivug_db_create_filter(&media_filter);
	ret = media_tag_foreach_tag_from_db(media_filter, _media_tag_list_cb, iv_db);

	free(iv_db);

	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_tag_foreach_tag_from_db is failed, err = %s", _strerror_db(ret));
		goto GET_ITEM_ERROR;
	}
	ivug_db_destroy_filter(media_filter);

	return true;

GET_ITEM_ERROR:
	if (media_filter)
		ivug_db_destroy_filter(media_filter);

	return false;
}

char *ivug_db_get_tag_name(tag_handle media)
{
	media_tag_h tag_item = (media_tag_h)media;
	char *name = NULL;

	media_tag_get_name(tag_item, &name);

	return name;
}

int ivug_db_get_tag_id(tag_handle media)
{
	media_tag_h tag_item = (media_tag_h)media;
	int id = 0;

	media_tag_get_tag_id(tag_item, &id);

	return id;
}

bool ivug_db_create_tag(tag_handle *tag_h, const char *tag_name)
{
	media_tag_h *tag = (media_tag_h *)tag_h;

	int ret = media_tag_insert_to_db(tag_name, tag);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_tag_insert_to_db, err = %s", _strerror_db(ret));
		*tag = NULL;
		return false;
	}

	ret = media_tag_update_to_db(*tag);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_tag_update_to_db, err = %s", _strerror_db(ret));
		media_tag_destroy(*tag);
		*tag = NULL;
		return false;
	}

	return true;
}

bool ivug_db_destroy_tag(tag_handle tag_h)
{
	media_tag_h tag = (media_tag_h)tag_h;

	int ret = media_tag_destroy(tag);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_tag_destroy, err = %s", _strerror_db(ret));
		return false;
	}

	return true;
}

bool ivug_db_set_tag(media_handle m_handle, tag_handle t_handle)
{
	media_tag_h tag = (media_tag_h)t_handle;

	char *media_id = NULL;

	int ret = media_info_get_media_id(m_handle, &media_id);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_media_id, err = %s", _strerror_db(ret));
		return false;
	}
	if (media_id == NULL)
	{
		MSG_ERROR("media_info_get_media_id, media_id = NULL");
		return false;
	}

	ret = media_tag_add_media(tag, (const char *)media_id);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_tag_add_media, err = %s", _strerror_db(ret));
		if (media_id != NULL) {
			free(media_id);
			media_id = NULL;
		}
		return false;
	}

	ret = media_tag_update_to_db(tag);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_tag_update_to_db, err = %s", _strerror_db(ret));
		MSG_ERROR("or media tag already inserted");
	}
	free(media_id);
	media_id = NULL;
	return true;
}

bool ivug_db_update(media_handle media)
{
	media_info_h file_item = (media_info_h)media;

	int ret = media_info_update_to_db (file_item);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_update_to_db, err = %s", _strerror_db(ret));
		return false;
	}
	return true;
}

bool ivug_db_refresh(const char *media_id)
{
	int ret = media_info_refresh_metadata_to_db(media_id);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_refresh_metadata_to_db, err = %s", _strerror_db(ret));
		return false;
	}
	return true;
}

static bool _media_tag_item_cb(media_tag_h item, void *user_data)
{
	tag_handle *t_handle = (tag_handle *)user_data;

	media_tag_clone((media_tag_h *)t_handle, item);

	return false;	//only 1 item
}

tag_handle ivug_db_get_tag_handle(const char* tagname)
{
	IV_ASSERT(tagname != NULL);

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_handle media_filter = NULL;

	char buf[1024] = {0,};
	snprintf(buf, sizeof(buf), "TAG_NAME=\"%s\"", tagname);

	tag_handle t_handle = NULL;

	ivug_db_create_filter(&media_filter);
	ivug_db_set_filter_condition(media_filter, buf);
	ret = media_tag_foreach_tag_from_db(media_filter, _media_tag_item_cb, &t_handle);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_tag_foreach_tag_from_db is failed, err = %s", _strerror_db(ret));
		goto GET_ITEM_ERROR;
	}
	ivug_db_destroy_filter(media_filter);

	return t_handle;

GET_ITEM_ERROR:
	if (media_filter)
		ivug_db_destroy_filter(media_filter);

	return NULL;
}

bool ivug_db_set_favorite(media_handle media, bool set)
{
	media_info_h item = (media_info_h)media;

	int ret = media_info_set_favorite(item, set);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_set_favorite, err = %s", _strerror_db(ret));
		return false;
	}

	ret = media_info_update_to_db(item);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_update_to_db, err = %s", _strerror_db(ret));
		return false;
	}

	return true;
}

bool ivug_db_get_favorite(media_handle media, bool *bFavorite)
{
	media_info_h item = (media_info_h)media;

	int ret =  media_info_get_favorite(item, bFavorite);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		MSG_ERROR("media_info_get_favorite, err = %s", _strerror_db(ret));
		return false;
	}

	return true;
}

bool ivug_db_get_file_size(media_handle media, unsigned long long *size)
{
	media_info_h item = (media_info_h)media;

	int ret = media_info_get_size(item, size);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_size, err = %s", _strerror_db(ret));
		return false;
	}

	return true;
}

Ivug_DB_h * ivug_db_create_thumbnail(media_handle media, ivug_db_path_cb callback, void *data)
{
	media_info_h item = (media_info_h)media;

	Ivug_DB_h *db_h = (Ivug_DB_h *)calloc(1, sizeof(Ivug_DB_h));
	IV_ASSERT(db_h != NULL);
	db_h->m_handle = media;
	db_h->callback = callback;
	db_h->data = data;
	db_h->key = DB_KEY;

	int ret = media_info_create_thumbnail(item, _ivug_thumb_cb, db_h);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_create_thumbnail, err = %s", _strerror_db(ret));
		free(db_h);
		return NULL;
	}

	return db_h;
}

bool ivug_db_cancel_thumbnail(Ivug_DB_h *db_h)
{
	media_info_h item = (media_info_h)db_h->m_handle;

	int ret = media_info_cancel_thumbnail(item);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_cancel_thumbnail, err = %s", _strerror_db(ret));
		return false;
	}

	db_h->m_handle = NULL;
	db_h->callback = NULL;
	db_h->data = NULL;
	db_h->key = 0;

	free(db_h);

	return true;
}

bool ivug_db_get_modified_time(media_handle media, time_t *time)
{
	media_info_h item = (media_info_h)media;

	int ret = media_info_get_modified_time(item, time);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_modified_time, err = %s", _strerror_db(ret));
		return false;
	}
	return true;
}

bool ivug_db_get_time(media_handle media, time_t *time)
{
	media_info_h item = (media_info_h)media;

	int ret = media_info_get_timeline(item, time);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_timeline, err = %s", _strerror_db(ret));
		return false;
	}
	return true;
}

char *ivug_db_get_mime_type(media_handle media)
{
	media_info_h item = (media_info_h)media;

	char *mime = NULL;

	int ret = media_info_get_mime_type(item, &mime);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_mime_type, err = %s", _strerror_db(ret));
		return NULL;
	}

	MSG_MED("mime is %s", mime);

	return mime;
}

bool ivug_db_get_video_duration(media_handle media, int *duration)
{
	IV_ASSERT(duration != NULL);

	media_info_h item = (media_info_h)media;

	video_meta_h video_meta = NULL;

	int ret = media_info_get_video(item, &video_meta);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_video, err = %s", _strerror_db(ret));
		return false;
	}

	ret = video_meta_get_duration(video_meta, duration);
	video_meta_destroy(video_meta);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("video_meta_get_duration, err = %s", _strerror_db(ret));
		return false;
	}

	MSG_HIGH("duration is %d", *duration);

	return true;
}

Ivug_DB_h *ivug_db_set_updated_callback(ivug_db_update_cb callback, void *data)
{
	Ivug_DB_h *db_h = (Ivug_DB_h *)calloc(1, sizeof(Ivug_DB_h));
	IV_ASSERT(db_h != NULL);

	db_h->update_cb = callback;
	db_h->data = data;
	db_h->key = DB_KEY;

	//media_content_noti_h noti_handle = NULL;

//	int ret = media_content_set_db_updated_multiple_cb(&noti_handle, _db_update_cb, db_h);
//
//	if (ret != MEDIA_CONTENT_ERROR_NONE)
//	{
//		MSG_ERROR("media_content_set_db_updated_cb, err = %s", _strerror_db(ret));
//		return NULL;
//	}
//
//	MSG_ERROR("nhande 1 0x%08x, nhande 2 0x%08x", db_h->n_handle, noti_handle);
//
//	db_h->n_handle = (void *)noti_handle;

	//return db_h;

	free(db_h);
	return NULL;
}

bool ivug_db_unset_updated_callback(Ivug_DB_h *db_h)
{
	MSG_ERROR("nhande 1 0x%08x", db_h->n_handle);

//	int ret = media_content_unset_db_updated_multiple_cb((media_content_noti_h)db_h->n_handle);
//	if (ret != MEDIA_CONTENT_ERROR_NONE)
//	{
//		MSG_ERROR("media_content_unset_db_updated_cb, err = %s", _strerror_db(ret));
//		return false;
//	}
//
//	db_h->n_handle = NULL;
//	db_h->m_handle = NULL;
//	db_h->update_cb = NULL;
//	db_h->callback = NULL;
//	db_h->data = NULL;
//	db_h->key = 0;
//
//	free(db_h);

	return false;
}

bool ivug_db_get_taken_time(media_handle media, char **taken_time)
{
	image_meta_h image_m = NULL;
	media_info_h item = (media_info_h)media;

	int ret = media_info_get_image(item, &image_m);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_image, err = %s", _strerror_db(ret));
		return false;
	}

	ret = image_meta_get_date_taken(image_m, taken_time);
	image_meta_destroy(image_m);
	if (ret != MEDIA_CONTENT_ERROR_NONE || *taken_time == NULL)
	{
		MSG_ERROR("image_meta_get_date_taken, err = %s", _strerror_db(ret));
		return false;
	}

	MSG_HIGH("taken time is %s", *taken_time);

	return true;
}

bool ivug_db_get_location(media_handle media, char **location)
{
	MSG_ASSERT(location != NULL);

	media_info_h item = (media_info_h)media;

	int ret = media_info_get_location_tag(item, location);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("ivug_db_get_location, err=%s", _strerror_db(ret));
		return false;
	}

	if (*location == NULL)
	{
		MSG_WARN("Location is NULL for mHandle(0x%08x)", media);
		return false;
	}

	return true;
}

bool ivug_db_get_get_longitude(media_handle media, double * longitude)
{
	media_info_h item = (media_info_h)media;
	int ret = media_info_get_longitude(item, longitude);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("ivug_db_get_get_longitude, err=%s",_strerror_db(ret));
		return false;
	}

	return true;
}

bool ivug_db_get_get_latitude(media_handle media, double * latitude)
{
	media_info_h item = (media_info_h)media;
	int ret = media_info_get_latitude(item, latitude);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("ivug_db_get_get_latitude, err=%s",_strerror_db(ret));
		return false;
	}

	return true;
}

bool ivug_db_set_best_photo(const char *filepath)
{
//	IV_ASSERT(filepath != NULL);
//
//	MSG_SEC("bestphoto path is %s", filepath);
//
//	int ret = media_info_set_mode(filepath, MEDIA_CONTENT_MODE_BEST_PHOTO);
//	if (ret != MEDIA_CONTENT_ERROR_NONE)
//	{
//		MSG_ERROR("media_info_set_mode, err=%s", _strerror_db(ret));
//		return false;
//	}

	return false;
}

bool ivug_db_get_media_type(media_handle media, Ivug_Media_Type *type)
{
	IV_ASSERT(type != NULL);

//	media_info_h item = (media_info_h)media;
//
//	media_content_mode_e mode;
//
//	int ret = media_info_get_mode(item, &mode);
//	if (ret != MEDIA_CONTENT_ERROR_NONE)
//	{
//		MSG_ERROR("media_info_get_mode, err=%s", _strerror_db(ret));
//		*type = IV_MEDIA_TYPE_NORMAL;
//		return false;
//	}
//
//	MSG_MED("Media=0x%08x type is %d", media, mode);
//
//	switch (mode)
//	{
//		case MEDIA_CONTENT_MODE_NORMAL:
//			*type = IV_MEDIA_TYPE_NORMAL;
//			break;
//		case MEDIA_CONTENT_MODE_PANORAMA:
//			*type = IV_MEDIA_TYPE_PANORAMA;
//			break;
//		case MEDIA_CONTENT_MODE_SOUNDNSHOT:
//			*type = IV_MEDIA_TYPE_SOUNDNSHOT;
//			break;
//		case MEDIA_CONTENT_MODE_ANIMATED_PHOTO:
//			*type = IV_MEDIA_TYPE_ANIMATED_PHOTO;
//			break;
//		case MEDIA_CONTENT_MODE_BEST_PHOTO:
//			*type = IV_MEDIA_TYPE_BEST_PHOTO;
//			break;
//		case MEDIA_CONTENT_MODE_FACEBOOK:
//			*type = IV_MEDIA_TYPE_FACEBOOK;
//			break;
//		default:
//			*type = IV_MEDIA_TYPE_NORMAL;
//			break;
//	}

	return false;
}

bool ivug_db_is_burstshot(media_handle media)
{
	if (media == NULL)
	{
		MSG_WARN("IsBurst : Media handle is NULL");
		return false;
	}

	media_info_h item = (media_info_h)media;

	image_meta_h image_m = NULL;

	int ret = media_info_get_image(item, &image_m);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_image, err = %s", _strerror_db(ret));
		return false;
	}

	bool bBurstshot;

	ret = image_meta_is_burst_shot(image_m, &bBurstshot);

	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("image_meta_is_burstshot is failed. %s(0x%08x)", _strerror_db(ret), ret);
		image_meta_destroy(image_m);
		return false;
	}

// Example codes.
	if (bBurstshot == true)
	{
/*
	Burst ID is start from "1".
*/
		char *burstID;

		if (image_meta_get_burst_id(image_m, &burstID) != MEDIA_CONTENT_ERROR_NONE)
		{
			MSG_ERROR("Cannot get burst ID");
			image_meta_destroy(image_m);
			return false;
		}

		MSG_HIGH("Burst ID=%s", burstID);
		free(burstID);
	}

	image_meta_destroy(image_m);

	return bBurstshot;

}

char *ivug_db_get_burstID(media_handle media)
{
	media_info_h item = (media_info_h)media;

	image_meta_h image_m = NULL;
	char *burstID = NULL;

	int ret = media_info_get_image(item, &image_m);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("media_info_get_image, err = %s", _strerror_db(ret));
		return NULL;
	}

	ret = image_meta_get_burst_id(image_m, &burstID);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
	{
		MSG_ERROR("image_meta_get_burst_id is failed. %s(0x%08x)", _strerror_db(ret), ret);
		image_meta_destroy(image_m);
		return NULL;
	}

	MSG_HIGH("Burst ID=%s", burstID);

	image_meta_destroy(image_m);

	return burstID;
}

bool ivug_db_is_supported_file_type(media_handle media)
{
	bool ret = false;
	char *mime_type = NULL;
	mime_type = ivug_db_get_mime_type(media);
	if (mime_type == NULL)
	{
		MSG_SDATA_WARN("mime is not vaild");
		return false;
	}

	//image
	if (strncmp(mime_type, "image/jpeg", strlen("image/jpeg")) == 0
		|| strncmp(mime_type, "image/bmp", strlen("image/bmp")) == 0
		|| strncmp(mime_type, "image/png", strlen("image/png")) == 0
		|| strncmp(mime_type, "image/gif", strlen("image/gif")) == 0
		|| strncmp(mime_type, "image/vnd.wap.wbmp", strlen("image/vnd.wap.wbmp")) == 0)
	{
		ret = true;
	}
	else if (strncmp(mime_type, "video/", strlen("video/")) == 0)
	{
		ret = true;
	}
	else
	{
		MSG_SDATA_WARN("not supported file type");
		ret = false;
	}

	free(mime_type);
	return ret;
}

