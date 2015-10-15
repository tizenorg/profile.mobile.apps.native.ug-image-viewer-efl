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

#include "ivug-datatypes.h"
#include "ivug-filter.h"

#include <Eina.h>

#include "ivug-debug.h"
#include "ivug-uuid.h"
#include "ivug-util.h"

void ivug_data_filter_delete(Filter_struct *filter_str)
{
	IV_ASSERT(filter_str != NULL);

	MSG_SDATA_HIGH("Removing filter struct. pFilter=0x%08x Type=%d", filter_str, filter_str->type);

	if (filter_str->selected_list)
	{
		eina_list_free(filter_str->selected_list);
		filter_str->selected_list = NULL;
	}

	if (filter_str->type == FILTER_DB)
	{
		DB_Filter *filter = filter_str->db_filter;
		switch (filter_str->view_by)
		{
		case IVUG_VIEW_BY_PLACES:
			break;
		case IVUG_VIEW_BY_TIMELINE:
			break;
		case IVUG_VIEW_BY_TAG:
			break;
		case IVUG_VIEW_BY_FAVORITES:
			break;

		case IVUG_VIEW_BY_FILE:
			if (filter->file_path)
			{
				free(filter->file_path);
			}
			break;

		case IVUG_VIEW_BY_ALL:
			uuid_free(filter->album_id);
			break;

		case IVUG_VIEW_BY_HIDDEN_ALL:
			uuid_free(filter->album_id);
			break;

		case IVUG_VIEW_BY_FOLDER:
			uuid_free(filter->album_id);
			break;

		case IVUG_VIEW_BY_HIDDEN_FOLDER:
			uuid_free(filter->album_id);
			break;
		case IVUG_VIEW_BY_INVAILD:
		default:
			MSG_SDATA_WARN("Invalid ViewBy : %d", filter_str->view_by);
			break;
		}
		free(filter);
	}
	else if (filter_str->type == FILTER_DIRECTORY)
	{
		Direcotry_Filter *dir_filter = filter_str->dir_filter;
		if (dir_filter)
		{
			free((char *)dir_filter->basedir);
			free((char *)dir_filter->current);
			free(dir_filter);
		}
	}
	if (filter_str->filepath)
	{
		free(filter_str->filepath);
		filter_str->filepath = NULL;
	}

	free(filter_str);
}


Filter_struct *ivug_data_filter_copy(const Filter_struct *filter_str)
{
	IV_ASSERT(filter_str != NULL);
	Filter_struct *cFilter_str = calloc(1, sizeof(Filter_struct));
	IV_ASSERT(cFilter_str != NULL);

	memcpy(cFilter_str, filter_str, sizeof(Filter_struct));

	if (filter_str->type == FILTER_DB)
	{
		DB_Filter *filter = filter_str->db_filter;
		DB_Filter *cFilter = calloc(1, sizeof(DB_Filter));
		IV_ASSERT(cFilter != NULL);
		memcpy(cFilter, filter, sizeof(DB_Filter));

		switch (filter_str->view_by)
		{
		case IVUG_VIEW_BY_PLACES:
			break;
		case IVUG_VIEW_BY_TIMELINE:
			break;
		case IVUG_VIEW_BY_TAG:
			cFilter->tag_id = filter->tag_id;
			break;
		case IVUG_VIEW_BY_FAVORITES:
			break;

		case IVUG_VIEW_BY_FILE:
			cFilter->file_path = strdup(filter->file_path);
			break;

		case IVUG_VIEW_BY_ALL:
			cFilter->album_id = uuid_assign(filter->album_id);
			break;

		case IVUG_VIEW_BY_HIDDEN_ALL:
			cFilter->album_id = uuid_assign(filter->album_id);
			break;

		case IVUG_VIEW_BY_FOLDER:
			cFilter->album_id = uuid_assign(filter->album_id);
			break;

		case IVUG_VIEW_BY_HIDDEN_FOLDER:
			cFilter->album_id = uuid_assign(filter->album_id);
			break;
		case IVUG_VIEW_BY_INVAILD:
		default:
			MSG_SDATA_WARN("Invalid ViewBy : %d", filter_str->view_by);
			break;
		}

		cFilter_str->db_filter = cFilter;
	}

	if (filter_str->filepath)
	{
		cFilter_str->filepath = strdup(filter_str->filepath);
	}
	return cFilter_str;
}
