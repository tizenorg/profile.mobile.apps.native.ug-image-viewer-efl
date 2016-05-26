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

#include <Elementary.h>
#include <app.h>

#include "ivug-debug.h"
#include "ivug-define.h"

#include "ivug-base.h"


/*
	When turn on TRACE_MEMORY, you should set sticky bit to memsp.

	#chmod 4777 /usr/bin/memps
*/
#undef TRACE_MEMORY

#ifdef TRACE_MEMORY
#include <unistd.h>
#endif

static int nRunCount = 0;

#define _CONSTRUCTOR __attribute__ ((constructor))
#define _DESTRUCTOR __attribute__ ((destructor))

_CONSTRUCTOR void _DLLInit(void)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer - Called DLL constructor");
}

_DESTRUCTOR void _DLLExit(void)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer - Called DLL destructor");
}

int main(int argc, char *argv[])
{

	struct _ug_data ugd;

	MSG_IMAGEVIEW_HIGH("IMAGE_VIEWER_MODULE ENTRANCE. Ver=12.0. RunCount=%d", nRunCount);

	ui_app_lifecycle_callback_s ops;

/*	int ret = APP_ERROR_NONE;

	app_event_handler_h hLanguageChangedHandle;
	app_event_handler_h hRegionFormatChangedHandle;
*/
	memset(&ops, 0x0, sizeof(ui_app_lifecycle_callback_s));
	memset(&ugd, 0x0, sizeof(struct _ug_data));

	nRunCount++;

	MSG_IMAGEVIEW_HIGH("IMAGE_VIEWER_MODULE REGISTER SYSTEM CALBBACKS");

	ops.create = on_create;
	ops.terminate = on_destroy;
	ops.pause = on_pause;
	ops.resume = on_resume;
	ops.app_control = ivug_param_create_from_bundle;

	/*ret = ui_app_add_event_handler(&hRegionFormatChangedHandle, APP_EVENT_REGION_FORMAT_CHANGED, _language_changed_cb, (void*)&ugd);
	if (ret != APP_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("APP_EVENT_REGION_FORMAT_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}

	ret = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, _language_changed_cb, (void*)&ugd);
	if (ret != APP_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", ret);
		return -1;
	}*/
	return ui_app_main(argc, argv, &ops, &ugd);
}
