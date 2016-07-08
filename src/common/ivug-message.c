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

#include "ivug-message.h"

#include <sys/types.h>	// gettid()
#include <pthread.h>

#include <Eina.h>
#include <Ecore.h>

#undef UNIT_TEST

#ifdef UNIT_TEST
/*
	Build gcc fmradio_message.c `pkg-config --cflags --libs eina dlog ecore ecore-input`
*/
#include <dlog.h>

#include <assert.h>
#include <Ecore_Input.h>

#undef LOG_TAG
#define LOG_TAG "UTT_MESSAGE"

#define MSG_ASSERT(expr) do { if (!(expr)) { LOG(LOG_INFO, LOG_TAG, "[%s] ASSERT : " #expr, __func__); assert(0); } }while (0)

#define MSG_FATAL(...) 	do { LOG(LOG_INFO, LOG_TAG, ##__VA_ARGS__); assert(0); } while (0)
#define MSG_WARN(...) 	LOG(LOG_INFO, LOG_TAG, ##__VA_ARGS__)
#define MSG_ERROR(...) 	LOG(LOG_INFO, LOG_TAG, ##__VA_ARGS__)
#define MSG_HIGH(...)	LOG(LOG_INFO, LOG_TAG, ##__VA_ARGS__)
#define MSG_MED(...)	LOG(LOG_INFO, LOG_TAG, ##__VA_ARGS__)
#else

#include "ivug-debug.h"

#define LOG_LVL DBG_MSG_LVL_HIGH
#define LOG_CAT "IV-MESSAGE"

#endif

#define MAX_COMMAND_LENGTH (30)		// Max length of each command

typedef struct {
	int param1;
	int param2;
	int param3;
	void *param4;

	char command[MAX_COMMAND_LENGTH];
} MyData;

typedef struct {
	FnMessage callback;
	void *client_data;

	bool delete_me;
} RegisteredCallback;

typedef struct {
	EINA_INLIST;

	char command[MAX_COMMAND_LENGTH];
	Eina_List/*<RegisteredCallback>*/ *cblist;
} CommandList;

typedef struct {
	Ecore_Pipe *pipe;

	Eina_Inlist *command_list;
	int registered_count;

	pthread_mutex_t pipe_mutex;

} _MessageHandle, *_PMessageHandle;

#define PRINT_ERRNO(errno, fmt, arg...) \
	do { \
		char szError[256]; \
		int perr; \
		perr = strerror_r(errno, szError, sizeof(szError)); \
		\
		if (perr) { \
			MSG_ERROR(fmt " %s", ##arg, szError); \
		} \
		else { \
			MSG_ERROR(fmt " Errno=%d", ##arg, errno); \
		} \
	} while (0)


RegisteredCallback *_find_callback(Eina_List *cb_list, FnMessage cb)
{
	Eina_List *l;
	RegisteredCallback *pCallback;

	EINA_LIST_FOREACH(cb_list,l, pCallback)
	{
		if ((pCallback->callback == cb))
		{
			return pCallback;
		}
	}

// Not found!.
	return NULL;
}


CommandList *_find_command(Eina_Inlist *command_list, const char *command)
{
	CommandList *l;

	EINA_INLIST_FOREACH(command_list, l)
	{
		if (strncmp(l->command, command, MAX_COMMAND_LENGTH) == 0)
		{
			return l;
		}
	}

	return NULL;
}


static void _cleanup(_PMessageHandle pmessage)
{
	CommandList *pData = NULL;
	Eina_Inlist *l2;

	EINA_INLIST_FOREACH_SAFE(pmessage->command_list, l2, pData)
	{
		RegisteredCallback *pCallback = NULL;
		Eina_List *l, *l_next;

		EINA_LIST_FOREACH_SAFE(pData->cblist, l, l_next, pCallback)
		{
			if (pCallback->delete_me == true)
			{
				pData->cblist = eina_list_remove_list(pData->cblist, l);
				pCallback->delete_me = false;
				free(pCallback);
				pmessage->registered_count--;
			}
		}

		if (eina_list_count(pData->cblist) == 0)
		{
			pmessage->command_list = eina_inlist_remove(pmessage->command_list, EINA_INLIST_GET(pData));

			MSG_HIGH("Remve cmd slot for %s", pData->command);
			free(pData);
			pData = NULL;
		}
	}

}


void pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	_PMessageHandle pmessage = (_PMessageHandle)data;
	MyData *mydata = buffer;

	if (nbyte != sizeof(MyData))
	{
		MSG_ERROR("Invalid command!!. pipe=0x%08x nByte=%d", pmessage->pipe, nbyte);
		return ;
	}

	MSG_MED("Read from pipe. Pipe=0x%08x nByte=%d Cmd=%s Param1=%d", pmessage->pipe, nbyte , mydata->command, mydata->param1);

//	dump_message(pmessage);

	CommandList *pData;

	pData = _find_command(pmessage->command_list, mydata->command);

// Clean up deleted callback
	if (pData != NULL)		// Add new command
	{
		RegisteredCallback *pCallback = NULL;
		Eina_List *l;

		EINA_LIST_FOREACH(pData->cblist, l, pCallback)
		{
			if (pCallback->delete_me == false)
			{
			pCallback->callback(mydata->param1, mydata->param2, mydata->param3, mydata->param4, pCallback->client_data);	// Call user function
		}
		}

		_cleanup(pmessage);
		return;
	}

	MSG_ERROR("Unknown command. pipe=0x%08x nByte=%d [%s:%d]", pmessage->pipe, nbyte, mydata->command, mydata->param1);
}


MessageHandle create_message_handle()
{
	_PMessageHandle pmessage = (_PMessageHandle) malloc(sizeof(_MessageHandle));

	if (pmessage == NULL)
	{
		MSG_FATAL("Cannot allocate memory. Size=%d", sizeof(_MessageHandle));
		return NULL;
	}

	memset(pmessage, 0x00, sizeof(_MessageHandle));

	if (pthread_mutex_init(&pmessage->pipe_mutex, NULL) != 0) // if falied,
	{
		PRINT_ERRNO(errno, "Mutex init error");

		free(pmessage);
		return NULL;
	}

	pmessage->pipe = ecore_pipe_add(pipe_cb, pmessage);

	if (pmessage->pipe == NULL)
	{
		MSG_ERROR("Failed to creating ecore pipe");
		if (pthread_mutex_destroy(&pmessage->pipe_mutex) != 0)
		{
			PRINT_ERRNO(errno, "Mutex destroy error");
			// Go through
		}

		free(pmessage);
		return NULL;
	}

	return (MessageHandle)pmessage;
}

bool remove_message_handle(MessageHandle handle)
{
	_PMessageHandle pmessage = (_PMessageHandle)handle;

	// this function can be called in signal handler. so assert() cannot be used in this function
	if (pmessage == NULL)
	{
		MSG_WARN("Message handle is NULL");
		return true;
	}

	MSG_HIGH("Remove message handle. Handle=0x%08x", handle);
	if (pmessage->pipe != NULL)
	{
		ecore_pipe_del(pmessage->pipe);
		pmessage->pipe = NULL;
	}

	if (pthread_mutex_destroy(&pmessage->pipe_mutex) != 0)
	{
		PRINT_ERRNO(errno, "Mutex destroy error");

		// Go through
	}

	CommandList *pData;

	EINA_INLIST_FOREACH(pmessage->command_list, pData)
	{
		RegisteredCallback *pCallback = NULL;
		EINA_LIST_FREE(pData->cblist, pCallback)
		{
			free(pCallback);
			pmessage->registered_count--;
		}
	}

	while (pmessage->command_list)
	{
		Eina_Inlist *aux = pmessage->command_list;
		pmessage->command_list = eina_inlist_remove(pmessage->command_list, pmessage->command_list);
		free(aux);
	}

	MSG_HIGH("Registered Count=%d", pmessage->registered_count);

	free(pmessage);

	return true;
}

bool send_message(MessageHandle handle, const char *command,  int param1, int param2, int param3, void *param4)
{
	_PMessageHandle pmessage = (_PMessageHandle)handle;

	if (pthread_mutex_lock(&pmessage->pipe_mutex) != 0)
	{
		PRINT_ERRNO(errno, "Mutex lock error");

		return false;
	}

	MyData data = {0,};

	data.param1 = param1;
	data.param2 = param2;
	data.param3 = param3;
	data.param4 = param4;

	strncpy(data.command, command, MAX_COMMAND_LENGTH-1);

	MSG_MED("Write to pipe. tID=0x%08x Pipe=0x%08x Cmd=%s Param1=%d", pthread_self(), pmessage->pipe, data.command, data.param1);

	MSG_ASSERT(pmessage->pipe != NULL);

	if (ecore_pipe_write(pmessage->pipe, &data, sizeof(MyData)) == EINA_FALSE)
	{
		MSG_ERROR("Writing to pipe is failed. pipe=0x%08x size=%d", pmessage->pipe, sizeof(MyData));

		if (pthread_mutex_unlock(&pmessage->pipe_mutex) != 0)
		{
			PRINT_ERRNO(errno, "Mutex unlock error");

			// Go through
		}

		return false;
	}

	if (pthread_mutex_unlock(&pmessage->pipe_mutex) != 0)
	{
		PRINT_ERRNO(errno, "Mutex unlock error");

		// Go through
	}

	return true;
}

void dump_message(MessageHandle handle)
{
	_PMessageHandle pmessage = (_PMessageHandle)handle;
	CommandList *pData;

	MSG_HIGH("*****************************");
	MSG_HIGH("  Total : %d CommandList=0x%08x", pmessage->registered_count, pmessage->command_list);

	EINA_INLIST_FOREACH(pmessage->command_list, pData)
	{
		MSG_HIGH("  Command : \"%s\" 0x%08x", pData->command, pData);

		RegisteredCallback *pCallback = NULL;
		Eina_List *l;

		EINA_LIST_FOREACH(pData->cblist, l, pCallback)
		{
			MSG_HIGH("    Callback=0x%08x, Data=0x%08x", pCallback->callback, pCallback->client_data);

		}
	}

	MSG_HIGH("*****************************");

}
