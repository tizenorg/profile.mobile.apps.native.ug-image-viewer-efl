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
#include "ivug-medialist.h"

#include "ivug-mediadata.h"

#include "ivug-data-list.h"


#include "ivug-db.h"

#include <Ecore.h>
#include <pthread.h>
#include <algorithm>


#undef DEBUG_DUMP_LIST

#define LOAD_BLOCK_COUNT (100)

#undef UNIT_TEST
#define UT_USE_DB					/* For DB unit test */
#define UT_TOTAL (50)
#define UT_INDEX (49)

#define USE_ECORE_CHECK

typedef struct {
	Eina_List *header; 	 // Start pointer for media list
	int count; 	 // Total count

	Ecore_Thread *thread;	// Loading thread
	Eina_Lock mutex;
	Eina_Condition condition;

	Eina_Bool bTerminate;	//

	bool bLoading;			// Is thread loading on progress?
	bool bStarted;			// Is thread started?
	bool bNeedUpdate;
	bool bHidden;			// Is hidden file list
	Filter_struct *filter_str;

	Eina_List *shufflelist;	// Shuffle liste

	Media_Item *cur_mitem;
	Media_Item *prev_mitem;

	Ecore_Job *callback_job;
	ivug_medialist_cb cb;	// loaded callback
	void *data;

	Ivug_DB_h *db_handle;

	void *drm_data;

	int lBound;		// Lower bound of loaded Index
	int uBound;		// Upper bound of loaded Index

	int MaxIndex;		// Max Index. [0, MaxIndex]
} _Media_List;


typedef struct {
	_Media_List *_mList;
	Filter_struct *filter_str;

	bool bCanceled;

	int lowerBnd;		// Lower bound of loaded Index
	int upperBnd;		// Upper bound of loaded Index
	int TotalCount;
} ThreadParam;

static int _db_updated_callback(media_handle media, const char *path, Ivug_DB_Update_Type type, void *user_data)
{
	_Media_List *_mList = (_Media_List *)user_data;

	if (type == IV_DB_UPDATE_UPDATE) {
		UUID uuid = ivug_db_get_file_id(media);

		Media_Item *mitem = ivug_medialist_find_item_by_uuid((Media_List*)_mList, uuid);

		uuid_free(uuid);

		if (mitem == NULL) {
			MSG_SDATA_HIGH("cannot find in media list, updated path = %s", path);
			return -1;
		}

		Media_Data *mdata = ivug_medialist_get_data(mitem);
		if (mdata == NULL) {
			MSG_SDATA_ERROR("mdata is NULL");
			return -1;
		}

		free(mdata->filepath);
		mdata->filepath = ivug_db_get_file_path(media);

		free(mdata->thumbnail_path);
		mdata->thumbnail_path = ivug_db_get_thumbnail_path(media);

		ivug_db_destroy_file_handle(media);
	} else if (type == IV_DB_UPDATE_INSERT) {
		//TODO later
		_mList->bNeedUpdate = true;
	} else if (type == IV_DB_UPDATE_DELETE) {
		//TODO later
		_mList->bNeedUpdate = true;
	}

	return 0;
}

static Eina_List *_load_partial(const Filter_struct *filter, int stp, int endp)
{
	Eina_List *header = NULL;

#ifdef UT_USE_DB
	{
		if (filter->selected_list) {
			//header = ivug_list_load_DB_items_list(filter, filter->selected_list);
			header = ivug_list_load_DB_items(filter, stp, endp);
		} else {
			header = ivug_list_load_DB_items(filter, stp, endp);
		}
	}
#else
	Eina_List *header = NULL;

	int i;
	Media_Data *data;

	for (i = stp; i <= endp; i++) {
		data = malloc(sizeof(Media_Data));
		data->index = i;

		header = eina_list_append(header, data);
	}
#endif

	MSG_SDATA_HIGH("Loaded : %d ~ %d", stp, endp);

	return header;
}

static void _job_send_cb(void *data)
{
	IV_ASSERT(data != NULL);

	_Media_List *_mList = (_Media_List *)data;

	_mList->callback_job = NULL;

	if (_mList->cb) {
		_mList->cb(_mList, _mList->data);
	}
}

void ivug_media_list_free(Media_List *mList)
{
	_Media_List *_mList = (_Media_List *)mList;
	IV_ASSERT(_mList != NULL);

	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		free(data);
		data = NULL;
	}
}

static bool _ivug_medialist_set_medialist_to_media_item(Media_List *mList)
{
	/*
		Need to optimize.
	*/
	_Media_List *_mList = (_Media_List *)mList;
	IV_ASSERT(_mList != NULL);

	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *mdata = (Media_Data *)data;
		mdata->p_mList = mList;
	}

	return true;
}

static void _call_loaded_callback(_Media_List *_mList)
{
	IV_ASSERT(_mList != NULL);

	if (_mList->callback_job) {
		ecore_job_del(_mList->callback_job);
	}

	_mList->callback_job = ecore_job_add(_job_send_cb, _mList);
}

static void _doLoad(Ecore_Thread *thread, _Media_List *_mList, const Filter_struct *filter, int Total, int lBound, int uBound)
{
	bool bRightEnd = false;
	bool bLeftEnd = false;
	int iter = 1;

	Eina_List *left;
	Eina_List *right;

	int stp, endp;

	MSG_SDATA_HIGH("Total=%d lBound=%d uBound=%d", Total, lBound, uBound);

	if (lBound == 0) {
		bLeftEnd = true;
	}

	if (uBound == Total - 1) {
		bRightEnd = true;
	}

	while ((bLeftEnd && bRightEnd) == false) {
// Do Right Loading
		if (bRightEnd == false) {
			stp  = uBound + 1;
			endp = std::min(uBound + LOAD_BLOCK_COUNT / 2, Total - 1) ;

			right = _load_partial(filter, stp, endp);
			uBound = endp;

			if (endp == Total - 1) {
				bRightEnd = true;
			}

#ifdef USE_ECORE_CHECK
			if (ecore_thread_check(thread) == EINA_TRUE) {	// if pending cancelation
				MSG_SDATA_HIGH("Check True");
				ivug_list_delete_items(right);
				break;
			}
#else
			if (_mList->bTerminate == EINA_TRUE) {
				MSG_SDATA_HIGH("Check True");
				ivug_list_delete_items(right);
				return;
			}
#endif
			IV_ASSERT(right != NULL);
			_mList->header = eina_list_merge(_mList->header, right);
			_ivug_medialist_set_medialist_to_media_item((Media_List *)_mList);
		}

// Do Left Loading
		if (bLeftEnd == false) {	// Check whether need load
			stp  = std::max(lBound - LOAD_BLOCK_COUNT / 2, 0);
			endp = lBound - 1;

			left = _load_partial(filter, stp, endp);
			lBound = stp;

			if (stp == 0) {
				bLeftEnd = true;
			}

#ifdef USE_ECORE_CHECK
			if (ecore_thread_check(thread) == EINA_TRUE) {	// if pending cancelation
				MSG_SDATA_HIGH("Check True");
				ivug_list_delete_items(left);
				break;
			}
#else
			if (_mList->bTerminate == EINA_TRUE) {
				MSG_SDATA_HIGH("Check True");
				ivug_list_delete_items(left);
				return;
			}
#endif
			IV_ASSERT(left != NULL);

			_mList->header = eina_list_merge(left, _mList->header);
			_ivug_medialist_set_medialist_to_media_item((Media_List *)_mList);

		}


		iter++;
		usleep(100);
	}

	MSG_SDATA_HIGH("EinaCount=%d Count=%d", eina_list_count(_mList->header), _mList->count);
}

static void loader_free(void *data)
{
	ThreadParam *pParam = (ThreadParam *)data;

	ivug_data_filter_delete(pParam->filter_str);

	free(pParam);

	MSG_SDATA_HIGH("Thread param freed.");
}

static void loader_heavy(void *data, Ecore_Thread *thread)
{
	MSG_SDATA_HIGH("******** Thread started. Ecore tID=0x%08x pthread tID=0x%08x ******** ", thread, pthread_self());
	ThreadParam *pParam = (ThreadParam *)data;

	IV_ASSERT(pParam != NULL);

#ifdef USE_ECORE_CHECK
	if (ecore_thread_check(thread) == EINA_TRUE) {	// // if pending cancelation
		MSG_SDATA_HIGH("Pending thread cancelation");
		return;
	}
#else
	if (pParam->_mList->bTerminate == EINA_TRUE) {
		MSG_SDATA_HIGH("Check True");
		return;
	}
#endif

	Eina_Bool ret = EINA_FALSE;

	ret = eina_lock_new(&pParam->_mList->mutex);
	if (ret == EINA_FALSE) {
		MSG_SDATA_FATAL("eina_lock_new failed");
	} else {
		ret = eina_condition_new(&pParam->_mList->condition, &pParam->_mList->mutex);
		if (ret == EINA_FALSE) {
			MSG_SDATA_FATAL("eina_condition_new failed");
		}
	}

	pParam->_mList->bStarted = true;

	MSG_SDATA_HIGH("Load : Begin");

	_doLoad(thread, pParam->_mList, pParam->filter_str, pParam->TotalCount , pParam->lowerBnd, pParam->upperBnd);

	MSG_SDATA_HIGH("Load : End");

	pParam->_mList->thread = NULL;

#ifdef USE_ECORE_CHECK
	if (ecore_thread_check(thread) == EINA_FALSE)
#else
	if (pParam->_mList->bTerminate == EINA_FALSE)
#endif
	{
		MSG_SDATA_HIGH("Inform loaded callback to user. Total=%d", pParam->_mList->count);
		_call_loaded_callback(pParam->_mList);
	} else {
		MSG_SDATA_HIGH("list is terminating, did not call loaded callback");
	}

	eina_lock_take(&pParam->_mList->mutex);
	eina_condition_signal(&pParam->_mList->condition);
	eina_lock_release(&pParam->_mList->mutex);

	loader_free(pParam);
}


static void loader_end(void *data, Ecore_Thread *thread)
{
//	ThreadParam *pParam = data;

	//  MSG_SDATA_HIGH("Thread Ended. EinaCount=%d Count=%d", eina_list_count(pParam->_mList), pParam->_mList->count);
	MSG_SDATA_HIGH("Thread Ended. Ecore tID=0x%08x", thread);

	//PERF_CHECK_END(LVL3, "Deffered loading");
}

static void loader_cancel(void *data, Ecore_Thread *thread)
{
//	ThreadParam *pParam = data;

	MSG_SDATA_HIGH("Thread canceled. Ecore tID=0x%08x", thread);

	//PERF_CHECK_END(LVL3, "Deffered loading");
}

static int _sort_cb(const void *d1, const void *d2)
{
	return (rand() % 4 - 2) ;
}

#if 0
static Media_Item *_find_item(Media_List *mList, int index)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *mdata = (Media_Data *)data;

		if (mdata->index == index) {
			return (Media_Item *)l;
		}
	}

	return NULL;
}
#endif

#ifdef DEBUG_DUMP_LIST
static void _print_shuffle(void *data)
{
	MSG_SDATA_HIGH("Item : %d", (int)data);
}

static void _dump_list(Eina_List *list, void (*func)(void *))
{
	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(list, l, data) {
		func(data);
	}

}
#endif

static void _ivug_medialist_set_filter(Media_List *mList, Filter_struct *filter)
{
	IV_ASSERT(mList != NULL);

	_Media_List *_mList = (_Media_List *)mList;

	_mList->filter_str = filter;
}

static void _create_shuffle_list(_Media_List *_mList)
{
	int i;
	_mList->shufflelist = NULL;
	for (i = 0; i < _mList->count; i++) {
		_mList->shufflelist = eina_list_append(_mList->shufflelist, (void *)i);
	}

	_mList->shufflelist = eina_list_sort(_mList->shufflelist, eina_list_count(_mList->shufflelist), _sort_cb);
}

Media_List *ivug_medialist_create()
{
	_Media_List *_mList = (_Media_List *)calloc(1, sizeof(_Media_List));
	IV_ASSERT(_mList != NULL);

	srand((unsigned)time(NULL));

	MSG_SDATA_HIGH("Create media list : 0x%08x", _mList);

	return (Media_List *)_mList;
}

bool ivug_medialist_set_callback(Media_List *mList, ivug_medialist_cb callback, void *data)
{
	IV_ASSERT(mList != NULL);

	_Media_List *_mList = (_Media_List *)mList;
	_mList->cb = callback;
	_mList->data = data;

	return true;
}

Media_Item * ivug_medialist_find_item_by_filename(Media_List *mList, const char* filepath)
{
	IV_ASSERT(mList != NULL);
	IV_ASSERT(filepath != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *mdata = (Media_Data *)data;
		if (strncmp(mdata->fileurl, filepath, strlen(mdata->fileurl)) == 0) {
			return (Media_Item *)l;
		}
	}
	MSG_SDATA_ERROR("Cannot find file path %s at list(0x%08x)", filepath, _mList->header);
	return NULL;
}

Media_Item * ivug_medialist_find_item_by_uuid(Media_List *mList, UUID uuid)
{
	IV_ASSERT(mList != NULL);
	IV_ASSERT(uuid != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *mdata = (Media_Data *)data;

		if (strncmp(mdata->mediaID, uuid, strlen(mdata->mediaID)) == 0) {
			return (Media_Item *)l;
		}
	}
	MSG_SDATA_ERROR("Cannot find file uuid %s at list", uuid);
	return NULL;
}

static Media_Item *_ivug_medialist_load_from_directory(Media_List *mList, const Filter_struct *filter)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	MSG_SDATA_HIGH("Create slide data list. index=%d", filter->index);

	if (_mList->header != NULL) {
		MSG_SDATA_FATAL("Header is not NULL");
		return NULL;
	}
//	_mList->filter = ivug_data_filter_copy(filter);

	PERF_CHECK_BEGIN(LVL3, "MediaList - Get list count");

	//_mList->count = ivug_list_get_dir_cnt(filter->dir_filter->basedir);
	_mList->header = ivug_list_load_dir_items(filter->dir_filter->basedir);

	_mList->count = eina_list_count(_mList->header);
	if (_mList->count == 0) {
		MSG_SDATA_ERROR("No file founded");
		PERF_CHECK_END(LVL3, "MediaList - Get list count");
		return NULL;
	}

	MSG_SDATA_HIGH("list count = %d", _mList->count);

	_ivug_medialist_set_medialist_to_media_item(mList);

// Find Current.
	Eina_List *l = NULL;
	void *data;
	Media_Item *cur = NULL;
	int i = 0;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *pData = (Media_Data *)data;

		pData->index = i;
		i++;

		if (strcmp(pData->fileurl, filter->dir_filter->current) == 0) {
			cur = (Media_Item *)l;
		}
	}
	PERF_CHECK_END(LVL3, "MediaList - Get list count");

	if (cur == NULL) {
		MSG_SDATA_ERROR("Not found current");
		return NULL;
	}

	return cur;
}

static void _ivug_media_load_list_thread(Media_List *mList, const Filter_struct *filter)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	int max_thread = ecore_thread_max_get();
	int avail_thread = ecore_thread_available_get();
	int active_thread = ecore_thread_active_get();

	MSG_SDATA_HIGH("Thread status. CPU=%d Max=%d Active=%d Available=%d", eina_cpu_count(), max_thread, active_thread, avail_thread);
	ecore_thread_max_set(16 * 2);

	max_thread = ecore_thread_max_get();
	avail_thread = ecore_thread_available_get();
	active_thread = ecore_thread_active_get();

	MSG_SDATA_HIGH("Thread status. CPU=%d Max=%d Active=%d Available=%d", eina_cpu_count(), max_thread, active_thread, avail_thread);

	if (avail_thread <= 0) {
		MSG_SDATA_WARN("Thread will be enter to queue");
	}

	ThreadParam *pParam = NULL;

	pParam = (ThreadParam *)malloc(sizeof(ThreadParam));
	if (pParam == NULL) {
		MSG_SDATA_ERROR("malloc ERROR");
		return;
	}

	pParam->filter_str = ivug_data_filter_copy(filter);
	pParam->_mList = _mList;
	pParam->bCanceled = false;

	pParam->lowerBnd = _mList->lBound;
	pParam->upperBnd = _mList->uBound;
	pParam->TotalCount = _mList->count;

	_mList->bLoading = true;

	MSG_SDATA_HIGH("Starting thread");

	//PERF_CHECK_BEGIN(LVL3, "Deffered loading");

	// do not use "thread end", "thread cancel" callback
	// it can be called after ug unloaded
	_mList->thread = ecore_thread_run(loader_heavy, NULL, NULL, pParam);

	MSG_SDATA_HIGH("Ecore Thread ID = 0x%08x", _mList->thread);
	MSG_SDATA_HIGH("Thread func addr = 0x%08x, 0x%08x, 0x%08x", loader_heavy, loader_end, loader_cancel);

	ecore_thread_local_data_add(_mList->thread, "pParam", pParam, loader_free, EINA_FALSE);
}

static Media_Item *_ivug_media_load_list(Media_List *mList, const Filter_struct *filter, int index /* [0 ~ _mList->count) */)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	int db_idx = index;
	Eina_List *header = NULL;

	if (_mList->count < db_idx) {
		MSG_SDATA_ERROR("Invalid param. TotalCount=%d, DbIndex=%d", _mList->count, db_idx);
		return NULL;
	}

	int lBound = 0;
	int uBound = _mList->count - 1;

	/*lBound = std::max( db_idx - LOAD_BLOCK_COUNT / 2, 0 );

	if (filter->view_by == IVUG_VIEW_BY_ALL) {
		uBound = _mList->count - 1;
	} else {
		uBound = std::min( db_idx + LOAD_BLOCK_COUNT / 2, _mList->count - 1) ;
	}*/

	MSG_SDATA_HIGH("LUBound %d,%d", lBound, uBound);

	MSG_SDATA_HIGH("Total=%d Current=%d Bound(%d~%d)", _mList->count, db_idx, lBound, uBound);

	PERF_CHECK_BEGIN(LVL3, "MediaList - load first block");

// Load Center
	header = _load_partial(filter, lBound, uBound);

	_mList->lBound = lBound;
	_mList->uBound = uBound;

	PERF_CHECK_END(LVL3, "MediaList - load first block");

	if (header == NULL) {
		//MSG_SDATA_FATAL("MediaList is NULL");
		MSG_SDATA_ERROR("MediaList is NULL");
		return NULL;
	}

	_mList->header = header;

	_ivug_medialist_set_medialist_to_media_item(mList);

// find current data;
	PERF_CHECK_BEGIN(LVL3, "MediaList - Find current");

	Eina_List *current = eina_list_nth_list(header, db_idx - lBound);

	PERF_CHECK_END(LVL3, "MediaList - Find current");

	if (current == NULL) {
		MSG_SDATA_HIGH("current is NULL");
		return NULL;
	}

	PERF_CHECK_BEGIN(LVL3, "MediaList - shuffle");

	_create_shuffle_list(_mList);

	PERF_CHECK_END(LVL3, "MediaList - shuffle");

	// _dump_list(_mList->shufflelist, _print_shuffle);
	//  MSG_SDATA_HIGH("ParamPath=%s CurrentPath=%s", param->filepath, _mList->current->filepath);

	if ((lBound == 0) && (uBound == (_mList->count - 1))) {
		MSG_SDATA_HIGH("Deffered loading is not needed. LoadedCount=%d", eina_list_count(header));
		_call_loaded_callback(_mList);
		return (Media_Item *)current;
	}

	_ivug_media_load_list_thread(mList, filter);

	MSG_SDATA_HIGH("Create slide data list END");

	return (Media_Item *)current;
}

static Media_Item *_ivug_medialist_load_default(Media_List *mList, const Filter_struct *filter)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	MSG_SDATA_HIGH("Create slide data list. Title index=%d", filter->index);

	if (_mList->header != NULL) {
		MSG_SDATA_FATAL("Header is not NULL");
		return NULL;
	}
//	_mList->filter = ivug_data_filter_copy(filter);

	if (filter->view_by == IVUG_VIEW_BY_HIDDEN_ALL || filter->view_by == IVUG_VIEW_BY_HIDDEN_FOLDER) {
		_mList->bHidden = true;
	}

	int db_idx = 0;

#ifdef UNIT_TEST
	_mList->count = UT_TOTAL;
	db_idx = UT_INDEX;
#else
	PERF_CHECK_BEGIN(LVL3, "MediaList - Get list count");

	if (filter->file_list) {
		_mList->count = eina_list_count(filter->file_list);
		if (_mList->count == 0) {
			MSG_SDATA_ERROR("No file founded");
			PERF_CHECK_END(LVL3, "MediaList - Get list count");
			return NULL;
		}
		PERF_CHECK_BEGIN(LVL3, "MediaList - load file list");
		_mList->header = ivug_list_load_file_list(filter, filter->file_list);	 // Load all
		PERF_CHECK_END(LVL3, "MediaList - load file list");
		if (_mList->header == NULL) {
			MSG_SDATA_ERROR("MediaList is NULL");
			PERF_CHECK_END(LVL3, "MediaList - Get list count");
			return NULL;
		}
		_ivug_medialist_set_medialist_to_media_item(mList);
		MSG_SDATA_HIGH("%d items loaded", eina_list_count(_mList->header));
		_create_shuffle_list(_mList);
		Media_Item *cur = ivug_medialist_find_item_by_filename(mList, filter->filepath);
		_call_loaded_callback(_mList);
		PERF_CHECK_END(LVL3, "MediaList - Get list count");
		return cur;
	}

	if ((filter->view_by == IVUG_VIEW_BY_FOLDER && filter->index == IVUG_INVALID_INDEX) || filter->selected_list) {
		_mList->count = ivug_list_get_item_cnt(filter);
		if (_mList->count == 0) {
			MSG_SDATA_ERROR("No file founded");
			PERF_CHECK_END(LVL3, "MediaList - Get list count");
			return NULL;
		}
		PERF_CHECK_BEGIN(LVL3, "MediaList - load all block");
		_mList->header = _load_partial(filter, 0, _mList->count - 1);	 // Load all
		PERF_CHECK_END(LVL3, "MediaList - load all block");
		if (_mList->header == NULL) {
			MSG_SDATA_ERROR("MediaList is NULL");
			PERF_CHECK_END(LVL3, "MediaList - Get list count");
			return NULL;
		}
		_ivug_medialist_set_medialist_to_media_item(mList);
		MSG_SDATA_HIGH("%d items loaded", eina_list_count(_mList->header));
		_create_shuffle_list(_mList);
		Media_Item *cur = ivug_medialist_find_item_by_filename(mList, filter->filepath);
		_call_loaded_callback(_mList);
		PERF_CHECK_END(LVL3, "MediaList - Get list count");
		return cur;
	} else {
		_mList->count = ivug_list_get_item_cnt(filter);
	}

	PERF_CHECK_END(LVL3, "MediaList - Get list count");

	MSG_SDATA_HIGH("Total item count=%d", _mList->count);
/*
	TitleIndex : ¿ÜºÎ(Gallery)¿¡¼­ È£ÃâµÇ´Â Index = [1~]
	mDataIndex : Mdata->index, db_index´Â [0~]
*/
	db_idx = filter->index - 1;
#endif

	Media_Item *current = _ivug_media_load_list(mList, filter, db_idx);

	return current;
}


Media_Item *ivug_medialist_load(Media_List *mList, Filter_struct *filter)
{
	Media_Item *current = NULL;

	_ivug_medialist_set_filter(mList, filter);

	switch (filter->view_by) {
	case IVUG_VIEW_BY_DIRECTORY :
		PERF_CHECK_BEGIN(LVL2, "media list load - directory");
		current = _ivug_medialist_load_from_directory(mList, filter);
		PERF_CHECK_END(LVL2, "media list load - directory");
		break;
	default:
		PERF_CHECK_BEGIN(LVL2, "media list load");
		current = _ivug_medialist_load_default(mList, filter);
		PERF_CHECK_END(LVL2, "media list load");
		break;
	}
	return current;
}

Media_Item *ivug_medialist_reload(Media_List *mList, Media_Item *current)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	MSG_SDATA_HIGH("ivug_medialist_reload");

	const Filter_struct *filter = _mList->filter_str;

	Eina_List *list = NULL;
	Eina_List *old_list = NULL;
	int count = 0;
	Media_Item *new_current = NULL;

	Media_Data *mdata = ivug_medialist_get_data(current);
	if (mdata == NULL) {
		MSG_SDATA_ERROR("mdata is NULL");
		return NULL;
	}

	int cur_index = mdata->index;

	bool bUseThread = false;

	switch (filter->view_by) {
	case IVUG_VIEW_BY_DIRECTORY :
		PERF_CHECK_BEGIN(LVL2, "media list load - directory");
		list = ivug_list_load_dir_items(filter->dir_filter->basedir);
		count = eina_list_count(list);
		PERF_CHECK_END(LVL2, "media list load - directory");
		break;
	case IVUG_VIEW_BY_FOLDER:
	case IVUG_VIEW_BY_HIDDEN_FOLDER:
		PERF_CHECK_BEGIN(LVL2, "media list load");
		count = ivug_list_get_item_cnt(filter);
		list = _load_partial(filter, 0, count - 1);	 // Load all
		PERF_CHECK_END(LVL2, "media list load");
		break;
	default: {
		PERF_CHECK_BEGIN(LVL2, "media list load");
		count = ivug_list_get_item_cnt(filter);

		int lBound;
		int uBound;

		lBound = std::max(cur_index - LOAD_BLOCK_COUNT / 2, 0);
		uBound = std::min(cur_index + LOAD_BLOCK_COUNT / 2, _mList->count - 1) ;

		list = _load_partial(filter, lBound, uBound);

		PERF_CHECK_END(LVL2, "media list load");

		if ((lBound != 0) && (uBound != (_mList->count - 1))) {
			bUseThread = true;
		}
	}
	break;
	}

	if (list == NULL) {
		MSG_SDATA_ERROR("new list is NULL");
		return NULL;
	}

	eina_list_free(_mList->shufflelist);
	_create_shuffle_list(_mList);

	old_list = _mList->header;
	_mList->header = list;
	_mList->cur_mitem = NULL;

	_mList->count = count;
	_mList->bNeedUpdate = false;

	_ivug_medialist_set_medialist_to_media_item(mList);

	new_current = ivug_medialist_find_item_by_uuid(mList, mdata->mediaID);
	if (new_current == NULL) {
		MSG_SDATA_ERROR("current is not exist at list");
		//TODO : free data
		return NULL;
	}

	ivug_list_delete_items(old_list);

	ivug_medialist_set_current_item(mList, new_current);

// thread loading
	if (bUseThread == true) {
		_ivug_media_load_list_thread(mList, filter);
	} else {
		_call_loaded_callback(_mList);
	}

	return new_current;
}


void
ivug_medialist_del(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;
	Eina_Bool ret = EINA_FALSE;

	MSG_SDATA_HIGH("Removing all media data. mList=0x%08x", mList);

	if (_mList->callback_job) {
		ecore_job_del(_mList->callback_job);
		_mList->callback_job = NULL;
	}

	ivug_medialist_del_update_callback(mList);

	_mList->cb = NULL;	// remove loaded callback

	if (_mList->thread) {
		MSG_SDATA_HIGH("1. Thread cancel. Ecore tID=0x%08x", _mList->thread);

		if (ecore_thread_cancel(_mList->thread) == EINA_TRUE) {
			MSG_SDATA_HIGH("Thread canceled");
		} else {
			MSG_SDATA_ERROR("Thread cancel failed.");
		}

		_mList->bTerminate = EINA_TRUE;

		if (_mList->bStarted) {
			ret = eina_condition_wait(&_mList->condition);
			if (ret == EINA_FALSE) {
				MSG_SDATA_ERROR("eina_condition_wait error");
			}
			ret = eina_lock_release(&_mList->mutex);
			if (ret == EINA_FALSE) {
				MSG_SDATA_ERROR("eina_lock_release error");
			}
			eina_condition_free(&_mList->condition);
			eina_lock_free(&_mList->mutex);

			ecore_thread_local_data_del(_mList->thread, "pParam");
		} else {
			MSG_SDATA_HIGH("Thread is not started");
		}

		_mList->thread = NULL;
		MSG_SDATA_HIGH("2. Thread cancel");
	}

	ivug_list_delete_items(_mList->header);
	_mList->header = NULL;

	eina_list_free(_mList->shufflelist);
	_mList->shufflelist = NULL;

	if (_mList->filter_str) {
		ivug_data_filter_delete(_mList->filter_str);
		_mList->filter_str = NULL;
	}

	_mList->bStarted = false;

	free(_mList);

}


int ivug_medialist_get_count(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	return _mList->count;
}

int ivug_medialist_get_index(Media_List *mList, Media_Item *item)
{
	IV_ASSERT(mList != NULL);

	Media_Data *pData;

	MSG_SDATA_HIGH("Loading is not finished");
	pData = (Media_Data *)eina_list_data_get((Eina_List *)item);

	return pData->index + 1;
}

Media_Item *ivug_medialist_get_first(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	return (Media_Item *)_mList->header;
}


Media_Item *ivug_medialist_get_last(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	return (Media_Item *)eina_list_last(_mList->header);
}

Media_Item *ivug_medialist_get_next(Media_List *mList, Media_Item *item)
{
	IV_ASSERT(mList != NULL);

	IV_ASSERT(item != NULL);
	Eina_List *next = eina_list_next((Eina_List *)item);

	return (Media_Item *)next;
}


Media_Item *ivug_medialist_get_prev(Media_List *mList, Media_Item *item)
{
	IV_ASSERT(mList != NULL);

	IV_ASSERT(item != NULL);
	Eina_List *prev = eina_list_prev((Eina_List *)item);

	return (Media_Item *)prev;

}

Media_Data *ivug_medialist_get_data(const Media_Item *item)
{
	IV_ASSERT(item != NULL);

	return (Media_Data *)eina_list_data_get((Eina_List *)item);
}


void ivug_medialist_delete_item(Media_List *mList, Media_Item *item, bool deleteItem)
{
	PERF_CHECK_BEGIN(LVL1, "MediaList - delete");

	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	Media_Data *pData = (Media_Data *)eina_list_data_get((Eina_List *)item);

	Eina_List *list = eina_list_next((Eina_List *)item);
	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(list, l, data) {
		Media_Data *mData = (Media_Data *)data;
		mData->index = mData->index - 1;
	}

	Media_Item *temp_prev_mitem = ivug_medialist_get_prev(_mList, item);
	Media_Item *temp_next_mitem = ivug_medialist_get_next(_mList, item);

	_mList->header = eina_list_remove_list(_mList->header, (Eina_List *)item);

	// Change index
	if (deleteItem == true) {
		if (ivug_mediadata_delete(pData) == false) {
			MSG_SDATA_HIGH("Cannot delete mediadata");
		}
	}
	ivug_free_mediadata(pData);

	_mList->prev_mitem = temp_prev_mitem;
	_mList->cur_mitem = temp_next_mitem;

//	_mList->prev_mitem = NULL;		// reset prev as NULL
//	_mList->cur_mitem = NULL;		// reset cur as NULL

	// Shuffle list?
	_mList->count--;

	Eina_List *found = eina_list_data_find_list(_mList->shufflelist, (void *)_mList->count);

	_mList->shufflelist = eina_list_remove(_mList->shufflelist, found);

	MSG_SDATA_HIGH("Item removed. Total=%d", _mList->count);

	PERF_CHECK_END(LVL1, "MediaList - delete");
}


Media_Item *ivug_medialist_get_random_item(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	int count = eina_list_count(_mList->header);

	if (count != 0) {
		return (Media_Item *)eina_list_nth_list(_mList->header, random() % count);
	} else {
		return NULL;
	}
}

#if 0
Media_Item *ivug_medialist_get_shuffle_item(Media_List *mList, Media_Item *item)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	Media_Data *pData = (Media_Data *)eina_list_data_get((Eina_List *)item);

	Eina_List *found = eina_list_data_find_list(_mList->shufflelist, (void *)pData->index);

	if (found != NULL) {
		Media_Item *next;
		Eina_List *iter;

		iter = eina_list_next(found);

		if (iter == NULL) {
			int nFirstIdx = (int)eina_list_data_get(_mList->shufflelist);

			next = _find_item(mList, nFirstIdx);

			MSG_SDATA_HIGH("End reached. rewind to first. Index=%d", nFirstIdx);

			return next;
		}

		do {
			next = _find_item(mList, (int)eina_list_data_get(iter));

			if (next == NULL) {
				MSG_SDATA_HIGH("Index : %d is not loaded", (int)eina_list_data_get(iter));
				_mList->shufflelist = eina_list_demote_list(_mList->shufflelist, iter);
			}

			iter = eina_list_next(found);

		} while (next == NULL);

		Media_Data *nData = (Media_Data *)eina_list_data_get((Eina_List *)next);

		MSG_SDATA_HIGH("Shuffle : %d->%d", pData->index, nData->index);

//		_dump_list(_mList->shufflelist, _print_shuffle);
		return next;
	}

	MSG_SDATA_ERROR("Cannot find data. Index=%d", pData->index);
	return NULL;
}
#endif

Media_Item *
ivug_medialist_find_item_by_index(Media_List *mList, int index)
{
	IV_ASSERT(mList != NULL);

	MSG_IVUG_MED("ivug_medialist_find_item_by_index %d", index);

	_Media_List *_mList = (_Media_List *)mList;

	Eina_List *l = NULL;
	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *mdata = (Media_Data *)data;
		if (mdata == NULL) {
			MSG_IVUG_ERROR("album list is NULL");
			break;
		}
		if (mdata->index == index) {
			return (Media_Item *)l;
		}
	}
	return NULL;
}

Media_Item * ivug_medialist_append_item(Media_List *mList, const char *filepath)
{
	IV_ASSERT(mList != NULL);

	_Media_List *_mList = (_Media_List *)mList;

	_mList->header = ivug_list_append_item(_mList->header, filepath);

	_ivug_medialist_set_medialist_to_media_item(mList);

	_mList->count++;

	int index = 0;
	Eina_List *l;
	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *mdata = (Media_Data *)data;

		mdata->index = index++;
	}

	return (Media_Item *)eina_list_last(_mList->header);
}

Media_Item * ivug_medialist_prepend_item(Media_List *mList, const char *filepath)
{
	IV_ASSERT(mList != NULL);

	_Media_List *_mList = (_Media_List *)mList;

	_mList->header = ivug_list_prepend_item(_mList->header, filepath);

	_ivug_medialist_set_medialist_to_media_item(mList);

	_mList->count++;

	int index = 0;
	Eina_List *l;

	void *data;

	EINA_LIST_FOREACH(_mList->header, l, data) {
		Media_Data *mdata = (Media_Data *)data;

		mdata->index = index++;
	}

	return (Media_Item *)_mList->header;
}

bool ivug_medialist_set_current_item(Media_List *mList, Media_Item *mitem)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	_mList->prev_mitem = _mList->cur_mitem;
	_mList->cur_mitem = mitem;

	return true;
}

Media_Item * ivug_medialist_get_current_item(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	return _mList->cur_mitem;
}


Media_Item * ivug_medialist_get_prev_item(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	return _mList->prev_mitem;
}


void ivug_medialist_set_update_callback(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	MSG_SDATA_HIGH("ivug_medialist_set_update_callback 0x%08x, handle 0x%08x", _db_updated_callback, _mList->db_handle);

	if (_mList->db_handle) {
		MSG_SDATA_WARN("update_callback already exist");
		return;
	}

	_mList->db_handle = ivug_db_set_updated_callback(_db_updated_callback, _mList);

	return;
}

void ivug_medialist_del_update_callback(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	MSG_SDATA_HIGH("ivug_medialist_del_update_callback 0x%08x, handle 0x%08x", _db_updated_callback, _mList->db_handle);

	if (_mList->db_handle == NULL) {
		MSG_SDATA_WARN("update_callback already removed");
		return;
	}

	ivug_db_unset_updated_callback(_mList->db_handle);
	_mList->db_handle = NULL;

	return;
}

bool ivug_medialist_need_update(Media_List *mList)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	MSG_SDATA_HIGH("update flag = %d", _mList->bNeedUpdate);

	return _mList->bNeedUpdate;
}

void ivug_medialist_set_update_flag(Media_List *mList, bool flag)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	MSG_SDATA_HIGH("update flag = %d", flag);

	_mList->bNeedUpdate = flag;

	return;
}

Eina_List *ivug_medialist_get_burst_item_list(Media_List *mList, const char *burst_id)
{
	IV_ASSERT(mList != NULL);
	_Media_List *_mList = (_Media_List *)mList;

	int total_cnt = ivug_list_get_burst_item_cnt(_mList->filter_str, burst_id);

	Eina_List *list = ivug_list_load_burst_items(_mList->filter_str, burst_id, 0, total_cnt - 1);

	return list;
}

void ivug_medialist_del_burst_item_list(Eina_List *list)
{
	ivug_list_delete_items(list);
}

Filter_struct * ivug_medialist_get_filter(Media_List *mList)
{
	IV_ASSERT(mList != NULL);

	_Media_List *_mList = (_Media_List *)mList;

	return ivug_data_filter_copy(_mList->filter_str);
}

