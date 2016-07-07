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


#include "ivug-context.h"


#define MAX_INSTALNCE (5)

#undef USE_DEBUG_DUMP_CONTEXT

typedef struct
{
	int index;

	Evas_Object *parent_win;
	Evas_Object *conform;
	Evas_Object *parent_layout;
// EFL theme
	Elm_Theme *th;

// Window Geometry
	int win_w, win_h;	//size
	int rot;		//window rotation value (0~360)

	callback_handle_t *callback_handle;

	language_handle_t language_handle;

	bool bDesktopMode;
	bool bDestroying;

	app_control_h app_control_handle;

// Store indicator status
	Elm_Win_Indicator_Opacity_Mode indi_o_mode;
	Elm_Win_Indicator_Mode indi_mode;
	Eina_Bool oMode;
	char ALBUM_INDEX[256];
} AppData;

static Eina_List *ContextList = NULL;

Elm_Theme*
gGetSystemTheme(void)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->th;
}

void
gGetSystemDimension(int *w, int *h)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);
	IV_ASSERT(w != NULL);
	IV_ASSERT(h != NULL);

	*w = ugContext->win_w;
	*h = ugContext->win_h;
}


void
gSetRotationDegree(int degree)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	ugContext->rot = degree;
}



int
gGetRotationDegree(void)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);
	return elm_win_rotation_get(ugContext->parent_win);
}

int gGetScreenWidth()
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->win_w;

}

int gGetScreenHeight()
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->win_h;

}

Evas_Object *
gGetCurrentWindow(void)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->parent_win;
}

Evas_Object *
gGetCurrentConformant(void)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->conform;
}

static
void ivug_set_indicator_overlap_mode(bool bOverlap)
{
	Evas_Object *conform = gGetCurrentConformant();
	IV_ASSERT(conform != NULL);

	if (bOverlap == true) {
		elm_object_signal_emit(conform, "elm,state,indicator,overlap", "");

		evas_object_data_set(conform, "overlap", (void *)EINA_TRUE);
	} else {
		elm_object_signal_emit(conform, "elm,state,indicator,nooverlap", "");

		evas_object_data_set(conform, "overlap", (void *)EINA_FALSE);
	}
}

callback_handle_t *
gGetCallbackHandle(void)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->callback_handle;
}

language_handle_t
gGetLanguageHandle(void)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->language_handle;
}

void gSetAlbumIndex(const char* val)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);
	if (val != NULL)
	{
		snprintf(ugContext->ALBUM_INDEX, 256, "%s",val);
	}
}
const char* gGetAlbumIndex()
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->ALBUM_INDEX;
}

bool gGetDestroying()
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->bDestroying;
}

void gSetDestroying(bool isDestroying)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	ugContext->bDestroying = isDestroying;
}

app_control_h gGetServiceHandle()
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	return ugContext->app_control_handle;
}

void gSetServiceHandle(app_control_h service)
{
	AppData *ugContext;
	ugContext = eina_list_data_get(ContextList);

	IV_ASSERT(ugContext != NULL);

	if (ugContext->app_control_handle)
	{
		app_control_destroy(ugContext->app_control_handle);
	}

	app_control_clone(&ugContext->app_control_handle, service);
}

static const char *szMode[] = { "Unknown", "Hide", "Show" };

static const char *szOpacity[] = {
	"Unknown",
	"Opaque",
	"Translucent",
	"Transparent",
};

static const char *szOverlap[] = {
	"Non-overlap",
	"Overlap",
};

static void _dump_context(const char *title)
{
#ifdef USE_DEBUG_DUMP_CONTEXT
	Eina_List *l;
	void *data;

	MSG_IVUG_HIGH("**** Context : %-15s **********", title);

	EINA_LIST_FOREACH(ContextList, l, data)
	{
		AppData *Context = (AppData *)data;

		MSG_IVUG_HIGH("  Idx=%d Context=0x%08x UG=0x%08x", Context->index, Context, Context->ug);
	}

	MSG_IVUG_HIGH("****************************************");
#endif
}


bool
ivug_context_init(Evas_Object *win, Evas_Object *conform)
{
	static int index = 0;

	AppData *Context = (AppData *)calloc(1, sizeof(AppData));
	if (Context == NULL)
	{
		MSG_IVUG_ERROR("Cannot allock memory");
		return false;
	}

	Context->index = ++index;
	Context->parent_win = win;
	Context->conform = conform;


	if (Context->parent_win)
	{
		Context->rot = elm_win_rotation_get(Context->parent_win);
	}
	else
	{
		MSG_IMAGEVIEW_WARN("Cannot get parent window. rotation angle set as 0");
		Context->rot = 0;
	}

	int w, h;

	evas_object_geometry_get(Context->parent_win, NULL, NULL, &w, &h);

//	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	Context->win_w = w;
	Context->win_h = h;

	Context->indi_mode = elm_win_indicator_mode_get(Context->parent_win);
	Context->indi_o_mode = elm_win_indicator_opacity_get(Context->parent_win);

	int overlap = (int)evas_object_data_get(conform, "overlap");

	if (overlap == 0)
	{
		Context->oMode = EINA_FALSE;
	}
	else
	{
		Context->oMode = EINA_TRUE;
	}

	MSG_IMAGEVIEW_HIGH("Screen WH(%dx%d) Indicator(%s,%s,%s)", w, h, szMode[Context->indi_mode], szOpacity[Context->indi_o_mode], szOverlap[Context->oMode]);

	PERF_CHECK_BEGIN(LVL2, "theme add");

	Context->th = elm_theme_new();

	IV_ASSERT(Context->th != NULL);

	elm_theme_set(Context->th, "tizen-QVGA-dark");	//set dark theme
	elm_theme_ref_set(Context->th, NULL);

	elm_theme_extension_add(Context->th, full_path(EDJ_PATH, "/ivug-custom.edj"));

	PERF_CHECK_END(LVL2, "theme add");

	ContextList = eina_list_prepend(ContextList, Context);

	MSG_IVUG_HIGH("Append to list. Context=0x%08x", Context);

#ifdef USE_NEW_DB_API
	PERF_CHECK_BEGIN(LVL2, "media svc connect");

	ivug_db_create();

	PERF_CHECK_END(LVL2, "media svc connect");
#endif

	Context->callback_handle = ivug_callback_register();
	if (Context->callback_handle == NULL)
	{
		MSG_IVUG_ERROR("ivug_callback_register error");
		goto ERROR;
	}

	Context->bDesktopMode = false;
	const char *profile = elm_config_profile_get();
	if (strcmp(profile,"desktop") == 0)
	{
		Context->bDesktopMode = true;
	}

	ivug_language_mgr_create(&(Context->language_handle));

	bindtextdomain("image-viewer", LOCALE_PATH);	//bind text domain

	_dump_context("Init");
	return TRUE;

/**** Error routine ****/
ERROR:

	if (Context->callback_handle)
	{
		ivug_callback_unregister(Context->callback_handle);
	}

#ifdef USE_NEW_DB_API
	ivug_db_destroy();
#endif

	Context = eina_list_data_get(ContextList);
	ContextList = eina_list_remove_list(ContextList, ContextList);

	if (Context->th)
	{
		elm_theme_extension_del(Context->th, full_path(EDJ_PATH,"/ivug-custom.edj"));
		elm_theme_free(Context->th);
	}

	MSG_IVUG_HIGH("Remove from list. Context=0x%08x", Context);

	free(Context);

	return false;
}


bool
ivug_context_deinit()
{
	AppData *Context = NULL;

	if (ContextList == NULL) {
		MSG_IVUG_ERROR("ContextList is NULL");
		return false;
	}

	_dump_context("Before DeInit");

	Eina_List *l = NULL;
	Eina_List *l_next = NULL;

	EINA_LIST_FOREACH_SAFE(ContextList, l, l_next, Context)
	{
		if (Context == NULL) {
			MSG_IVUG_ERROR("Context is NULL");
			return false;
		}
	}

	if (Context != NULL) {
		if (Context->language_handle) {
			ivug_language_mgr_destroy(Context->language_handle);
			Context->language_handle = NULL;
		}

		if (Context->callback_handle) {
			MSG_IVUG_HIGH("Removing Callback");
			ivug_callback_unregister(Context->callback_handle);
			Context->callback_handle = NULL;
		}

		if (Context->app_control_handle) {
			app_control_destroy(Context->app_control_handle);
			Context->app_control_handle = NULL;
		}
	}
#ifdef USE_NEW_DB_API
	PERF_CHECK_BEGIN(LVL2, "ivug_db_destroy");
	ivug_db_destroy();
	PERF_CHECK_END(LVL2, "ivug_db_destroy");
#endif

	PERF_CHECK_BEGIN(LVL2, "elm_theme_free");
	if (Context && Context->th) {
		elm_theme_extension_del(Context->th, full_path(EDJ_PATH, "/ivug-custom.edj"));
		elm_theme_free(Context->th);
	}

	PERF_CHECK_END(LVL2, "elm_theme_free");

	MSG_IVUG_HIGH("Remove from list. Context=0x%08x", Context);

	free(Context);

	_dump_context("After DeInit");

	return true;
}


void ivug_context_destroy_me(const char *file, int line)
{
	AppData *Context = NULL;
	Eina_List *list = NULL;

	EINA_LIST_FOREACH(ContextList, list, Context) {
		if (Context && Context->bDestroying == false) {
			break;
		}
	}

	char *fname = strrchr(file, '/');

	if (NULL != Context) {
		_dump_context("DestroyME");

		MSG_IMAGEVIEW_HIGH("Request to destory ug. from L(%d) %s", line, fname);
		// Store indicator mode
		elm_win_indicator_mode_set(Context->parent_win, Context->indi_mode);

// When app does not set opacity mode, unknown is returned in _get()
// W/IV-COMMON(5576): 0:00:00.003[F:ivug-context.c   L:  367][HIGH] Screen WH(720x1280) Indicator(Show,Unknown,Overlap) <--- From email.
// Woraround. If opacity is unknown, set as different.
		if (Context->indi_o_mode == ELM_WIN_INDICATOR_OPACITY_UNKNOWN) {
			Context->indi_o_mode = ELM_WIN_INDICATOR_OPAQUE;
		}

		elm_win_indicator_opacity_set(Context->parent_win, Context->indi_o_mode);

		if (Context->oMode == EINA_TRUE) {
			ivug_set_indicator_overlap_mode(true);
		} else {
			ivug_set_indicator_overlap_mode(false);
		}

		MSG_IMAGEVIEW_HIGH("Restore Indicator(%s,%s,%s)", szMode[Context->indi_mode], szOpacity[Context->indi_o_mode], szOverlap[Context->oMode]);

		ui_app_exit();
	} else {
		MSG_IMAGEVIEW_WARN("Context is NULL. from L(%d) %s", line, fname);
	}
}


