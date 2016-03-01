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

#ifndef __IVUG_MAINVIEW_PRIV_H__
#define __IVUG_MAINVIEW_PRIV_H__

#include "ivug-define.h"

#include "ivug-name-view.h"
#include "ivug-crop-view.h"

#include "ivug-slider-new.h"
#include "ivug-media.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _Ivug_MainView{
	Evas_Object *photocam; // ULC added
	Evas_Object *photocam2;
	Evas_Object *photocam0;
	Evas_Object *btn_favorite;
	int currentphotocam;
    int prevphotocam;
	bool  slide_state;
	Evas_Object *parent;
	Evas_Object *layout;		/*Layout widget based on EDJ*/

	Evas_Object *navi_bar;
	Evas_Object *lyContent;			// Content layout. contain toolbar, title, thumblist
	Elm_Object_Item *navi_it;

	Evas_Object *back_btn;
	int prev_mouse_point;
	int last_prev_mouse_point;

	bool is_moved;
	bool is_play_Icon;
	Ecore_Timer *slide_move_timer;
/*
   |------------------|
   | navi_bar		  |
   |------------------|
   | tool_bar		  |
   |------------------|
   |				  |
   |				  |
   |				  |
   |				  |
   |------------------|
*/

// Controlbar
	ivug_ctrlbar ctrl_bar_type;

	struct {
		Evas_Object *more;
		Evas_Object *share;
		Evas_Object *download;		// Save
		Evas_Object *nearby;
		Evas_Object *del;
		Evas_Object *info;
		Evas_Object *edit;
	} toolbtns;

#ifdef USE_THUMBLIST
	Evas_Object *thumbs;
#endif

	bool bSetThmByUser;
	bool bmultitouchsliding;

	Evas_Object* popup; 			//popup
	Evas_Object* ctx_popup; 		//context popup 1depth
	Evas_Object* ctx_popup2; 		//context popup 2depth

	Evas_Object* longpress_popup;		//long press popup
	bool bLongpressEnd;
	bool isSliding;

// child view.
	Ivug_NameView *pNameView;

	IvugCropView *pCropView;

// Media List
	Media_List *mList;

//for select view
	int total_selected;
	int max_count;
	long long int select_size;
	long long int limit_size;
	Evas_Object *select_bar;
	Evas_Object *check;
	Eina_List *selected_path_list;	// data is char *

//flag
	bool bShowMenu;

	ivug_mode mode;
	ivug_view_by view_by;
	ivug_media_type media_type;

	ui_gadget_h ext_ug;
	app_control_h ext_svc;

	Ecore_Event_Handler *keydown_handler;

	Ecore_Timer *exit_timer;
	Ecore_Timer *hide_timer;
	Ecore_Timer *back_timer;
	Ecore_Timer *reg_idler;
	Ecore_Timer *db_idler;
	int hide_count;

	Ecore_Timer *popup_timer;

	char *album_name;

// Slide show;
	SlideShow *ssHandle;
#ifdef USE_EXT_SLIDESHOW
	char *ss_music_name;
#endif

	bool bStandAlone;	//if true, it is process not ug

	bool bTestMode;
	bool bStartSlideshow;
	bool bPreventBackKey;		// prevent back key

	Ivug_SliderNew *pSliderNew;

	Ecore_Idler *delete_idler;
	Ecore_Thread *transcoding_thread;

	Eina_List *delete_list;
	int delete_total;
	Media_Item *cur_mitem;

	Evas_Object *progress_popup;

	Media_List *temp_mlist;	 //for selected slideshow

	Evas_Object *access_popup;	/* Tagbuddy Help popup*/
	Ecore_Timer *tagbuddy_idler;
	Media_Item *mCurrent;		// Do Not use for other purpose except for updating tagbuddy
};

void _update_favorite(struct _Ivug_MainView *pMainView);

bool ivug_is_agif(const char *filepath);

#ifdef __cplusplus
}
#endif


#endif 		// __IVUG_MAINVIEW_PRIV_H__

