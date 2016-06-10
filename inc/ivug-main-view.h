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

#ifndef __IVUG_MAIN_VIEWER_H__
#define __IVUG_MAIN_VIEWER_H__

#include "ivug-common.h"
#include "ivug-parameter.h"
#include "ivug-medialist.h"
#include "ivug-base.h"


#define MAX_CTRL_ITEM_CNT 5

//control bar type
typedef enum {
	CTRL_BAR_TYPE_UNDEFINED = 0x00,

	CTRL_BAR_TYPE_NONE,
	CTRL_BAR_TYPE_EMPTY,				// not exist ctrl bar
	CTRL_BAR_TYPE_SAVE,					// [save]
	CTRL_BAR_TYPE_FILE,					// [more,share,del]
	CTRL_BAR_TYPE_READ_ONLY,			// [more,down] web image
	CTRL_BAR_TYPE_ALLSHARE,				// [more,down] allshare image
	CTRL_BAR_TYPE_MAX
} ivug_ctrlbar;

typedef enum {
// only shows in ctrlbar
	TOOLBUTTON_DELETE,
	TOOLBUTTON_SHARE,
	TOOLBUTTON_ADDTAG,
	TOOLBUTTON_SLIDESHOW,
	TOOLBUTTON_SAVE,

// shows in either toolbar or ctrlbar
	TOOLBUTTON_EDIT,
	TOOLBUTTON_SETAS,
	TOOLBUTTON_DETAILS,
	TOOLBUTTON_TRIM,

// only shows in toolbar
	TOOLBUTTON_OK,
	TOOLBUTTON_CANCEL,

// Navigation hearder
	TOOLBUTTON_TOOL,

// NULL button for spacing
	TOOLBUTTON_NULL,
	TOOLBUTTON_MAX,
} ToolButtonType;

typedef enum {
	TOOLBUTTON_IN_NONE,
	TOOLBUTTON_IN_TOOLBAR,
	TOOLBUTTON_IN_CTRLBAR,
} TooButtonPos;

typedef struct {
	Elm_Object_Item *item;		// Control bar item in navigation header. ex) SetAs, Favorite, Info
	TooButtonPos pos;
} ControlBar_Item;
enum
{
    PHOTOCAM_0 = 0,
    PHOTOCAM_1 ,
    PHOTOCAM_2 ,
};	


typedef struct _SlideShow SlideShow;

typedef struct _Ivug_MainView Ivug_MainView;

#ifdef __cplusplus
extern "C" {
#endif

/*
	Create MainView layout
*/
void
on_slider_clicked(void *data, Evas_Object *obj, void *event_info);
Ivug_MainView *
ivug_main_view_create(Evas_Object* parent, ivug_parameter *param);

Evas_Object *
ivug_main_view_object_get(Ivug_MainView *pMainView);
int ivug_add_reg_idler(Ivug_MainView *pMainView);

void ivug_main_view_start(Ivug_MainView *pMainView, app_control_h service);


/*
	Load media list from parameter.
*/
bool ivug_main_view_set_list(Ivug_MainView *pMainView, ivug_parameter *ivug_param);

/*
	Start slide show.
*/
void ivug_main_view_start_slideshow(Ivug_MainView *pMainView, Eina_Bool bSlideFirst);

void ivug_main_view_resume(Ivug_MainView *pMainView);
void ivug_main_view_pause(Ivug_MainView *pMainView);

__attribute__((used)) void dump_obj(Evas_Object *obj, int lvl);

void ivug_main_view_destroy(Ivug_MainView *pMainView);

void ivug_main_view_reload(Ivug_MainView *pMainView);

void _update_main_view(Ivug_MainView *pMainView);

/*
	Enable/Disable Test mode.
	When test mode, application is termiated by pressing back-key
*/
void ivug_main_view_set_testmode(Ivug_MainView *pMainView, bool bTestMode);

/*
	Control GUI
*/
void ivug_main_view_show_menu_bar(Ivug_MainView *pMainView);
void ivug_main_view_hide_menu_bar(Ivug_MainView *pMainView);


void _ivug_main_view_del_hide_timer(Ivug_MainView *pMainView, const char *func, int line);

#define ivug_main_view_del_hide_timer(pMainView) _ivug_main_view_del_hide_timer(pMainView, __FUNCTION__, __LINE__)


void _ivug_main_view_set_hide_timer(Ivug_MainView *pMainView, const char *func, int line);

#define ivug_main_view_set_hide_timer(pMainView) _ivug_main_view_set_hide_timer(pMainView, __FUNCTION__, __LINE__)


void _ivug_main_on_mmc_state_changed(void *data);

#ifdef __cplusplus
}
#endif

#endif //__IVUG_MAIN_VIEWER_H__

