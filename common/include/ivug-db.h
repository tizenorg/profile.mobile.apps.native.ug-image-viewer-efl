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

#ifndef __IVUG_DB_H__
#define __IVUG_DB_H__

#include <stdbool.h>

#include "ivug-datatypes.h"
#include <time.h>		// localtime_r
#include "ivug-uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *filter_handle;
typedef void *media_handle;
typedef void *tag_handle;
typedef void *noti_handle;

typedef enum
{
	IV_DB_UPDATE_INSERT,
	IV_DB_UPDATE_DELETE,
	IV_DB_UPDATE_UPDATE,
	IV_DB_UPDATE_MAX
}Ivug_DB_Update_Type;

typedef bool (*ivug_db_cb)(media_handle media, void *user_data);
typedef void (*ivug_db_path_cb)(media_handle media, const char *path, void *user_data);
typedef int (*ivug_db_update_cb)(media_handle media, const char *path, Ivug_DB_Update_Type type, void *user_data);

#define IVUG_MAX_CONDITION_LEN (1024)

typedef struct _Ivug_DB_h
{
	ivug_db_path_cb callback;
	ivug_db_update_cb update_cb;
	media_handle *m_handle;
	noti_handle *n_handle;
	void *data;
	int key;
}Ivug_DB_h;

typedef enum
{
	IV_STORAGE_TYPE_INTERNAL,
	IV_STORAGE_TYPE_EXTERNAL,
	IV_STORAGE_TYPE_CLOUD,
	IV_STORAGE_TYPE_WEB,
	IV_STORAGE_TYPE_MAX
}Ivug_DB_Stroge_Type;

typedef enum
{
	IV_MEDIA_TYPE_NORMAL,
	IV_MEDIA_TYPE_PANORAMA,
	IV_MEDIA_TYPE_SOUNDNSHOT,
	IV_MEDIA_TYPE_ANIMATED_PHOTO,
	IV_MEDIA_TYPE_BEST_PHOTO,
	IV_MEDIA_TYPE_BURST,
	IV_MEDIA_TYPE_FACEBOOK,
	IV_MEDIA_TYPE_MAX
}Ivug_Media_Type;

bool ivug_db_create(void);
bool ivug_db_destroy(void);

bool ivug_db_update(media_handle media);
bool ivug_db_refresh(const char *media_id);
bool ivug_db_rename(media_handle m_handle, const char *dest);

bool ivug_db_create_filter(filter_handle *filter);
bool ivug_db_destroy_filter(filter_handle filter);
bool ivug_db_set_filter(filter_handle filter, ivug_view_by view_by, ivug_media_type media_type, char *condition);
bool ivug_db_set_image_time_asc_filter(filter_handle filter, char *condition);
bool ivug_db_set_filter_offset(filter_handle filter, int stp, int endp);

media_handle ivug_db_insert_file_to_DB(const char* filepath);

media_handle ivug_db_get_file_handle(const char* filepath);
media_handle ivug_db_get_file_handle_from_media_id(UUID media_id);
media_handle ivug_db_get_folder_handle(const char* folderpath);
tag_handle ivug_db_get_tag_handle(const char* tagname);

bool ivug_db_destroy_file_handle(media_handle m_handle);
bool ivug_db_destroy_folder_handle(media_handle m_handle);

bool ivug_db_get_all_folder_list(ivug_db_cb callback, void *data);
bool ivug_db_get_all_tag_list(ivug_db_cb callback, void *data);

bool ivug_db_create_tag(tag_handle *tag_h, const char *tag_name);
bool ivug_db_destroy_tag(tag_handle tag_h);
bool ivug_db_set_tag(media_handle m_handle, tag_handle t_handle);

bool ivug_db_set_favorite(media_handle media, bool set);
bool ivug_db_get_favorite(media_handle media, bool *bFavorite);

int ivug_db_get_tag_id(tag_handle media);



/*
	After use, uuid_free() for dealocate memory
*/
UUID ivug_db_get_file_id(media_handle media);
UUID ivug_db_get_folder_id(media_handle media);


/*
	below functions returned values should be freed by user
*/
char *ivug_db_get_file_path(media_handle media);

char *ivug_db_get_thumbnail_path(media_handle media);
char *ivug_db_get_folder_name(media_handle media);

char *ivug_db_get_folder_path(media_handle media);
char *ivug_db_get_tag_name(tag_handle media);
char *ivug_db_get_mime_type(media_handle media);
char *ivug_db_get_burstID(media_handle media);

bool ivug_db_get_file_size(media_handle media, unsigned long long *size);
bool ivug_db_get_modified_time(media_handle media, time_t *time);
bool ivug_db_get_time(media_handle media, time_t *time);
bool ivug_db_get_video_duration(media_handle media, int *duration);
bool ivug_db_get_folder_storage_type(media_handle media, Ivug_DB_Stroge_Type *type);
bool ivug_db_get_file_storage_type(media_handle media, Ivug_DB_Stroge_Type *type);

Ivug_DB_h * ivug_db_create_thumbnail(media_handle media, ivug_db_path_cb callback, void *data);
bool ivug_db_cancel_thumbnail(Ivug_DB_h *db_h);

Ivug_DB_h *ivug_db_set_updated_callback(ivug_db_update_cb callback, void *data);
bool ivug_db_unset_updated_callback(Ivug_DB_h *db_h);

bool ivug_db_get_taken_time(media_handle media, char **taken_time);

bool ivug_db_set_best_photo(const char *filepath);
bool ivug_db_get_media_type(media_handle media, Ivug_Media_Type *type);
bool ivug_db_get_location(media_handle media, char **location);
bool ivug_db_get_get_longitude(media_handle media, double * longitude);
bool ivug_db_get_get_latitude(media_handle media, double * latitude);

/*
	Returns whether given media is burstshot or not
*/
bool ivug_db_is_burstshot(media_handle media);

bool ivug_db_is_supported_file_type(media_handle media);

#ifdef __cplusplus
}
#endif


#endif // __IVUG_DB_H__

