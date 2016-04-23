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

#ifndef __IVUG_FILTER_H__
#define __IVUG_FILTER_H__

#include <Eina.h>
#include "ivug-define.h"

typedef enum {
	FILTER_DB,
	FILTER_DIRECTORY,
	FILTER_FILE_LIST,
	FILTER_TYPE_MAX
}filter_type_e;

typedef struct _DB_Filter{

	union {		/*	CAUTION : Union type. check view_by variable before free()*/
		char *file_path;
		struct {
			double max_longitude;
			double min_longitude;
			double max_latitude;
			double min_latitude;
		} place;

		struct {
			long start;
			long end;
		} time;

		int tag_id;

		UUID album_id;		// Cluster id
	};
} DB_Filter;

typedef struct _Directory_Filter {
	const char *basedir;
	const char *current;		// Center Item.
} Direcotry_Filter;

typedef struct _Filter_struct{
	filter_type_e type;
	ivug_view_by view_by;
	ivug_media_type media_type;
	ivug_sort_type sort_type;
	int index;
	char *filepath;

	Eina_List *selected_list;
	Eina_List *file_list;

	//union{
	DB_Filter *db_filter;
	Direcotry_Filter *dir_filter;
	//}
}Filter_struct;


#ifdef __cplusplus
extern "C" {
#endif

Filter_struct *ivug_data_filter_copy(const Filter_struct *filter);

void ivug_data_filter_delete(Filter_struct *filter);

#ifdef __cplusplus
}
#endif


#endif	// __IVUG_FILTER_H__

