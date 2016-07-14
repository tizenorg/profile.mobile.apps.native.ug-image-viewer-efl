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


#include "ivug-main-view.h"
#include "ivug-photocam.h"
#include "ivug-main-view-priv.h"
#include "ivug-main-view-toolbar.h"
#include "ivug-file-info.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define PC_MOVE_INTERVAL_TIME 0.02f

#undef LOG_LVL
#define LOG_LVL (DBG_MSG_LVL_HIGH | DBG_MSG_LVL_DEBUG)

#undef LOG_CAT
#define LOG_CAT "IV-PHOTOCAM"

/*initialize the values on finger touch to the screen*/
void _on_slider_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	on_slider_clicked(data, obj, event_info);
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;
	pMainView->prev_mouse_point = 0;
	pMainView->last_prev_mouse_point = ev->output.x;
	pMainView->is_moved = false;
	ivug_set_photocam_reset(pMainView->pSliderNew);
	MSG_MAIN_HIGH("mouse down (%d,%d)", ev->output.x, ev->output.y);
}
/*Move the photocam images along with the finger*/
void _on_slider_mouse_moved(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *) event_info;

	if (!ev->buttons) return;

	MSG_MAIN_HIGH("mouse moved current(%d,%d) prev(%d,%d)", ev->cur.output.x, ev->cur.output.y, ev->prev.output.x, ev->prev.output.y);
	int bx = 0;
	int by = 0;
	int bw = 0;
	int bh = 0;

	int count = -1;
	int currentindex = -1;
	if (pMainView->slide_state == true) {
		MSG_MAIN_HIGH("Sliding is happening");
		return ;//Do not accept the flick event  if transition is not completed
	}

	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);

	if (pMainView->cur_mitem && ivug_isslide_enabled(pMainView->pSliderNew)) {
		count = ivug_medialist_get_count(pMainView->mList);
		Media_Data *pData = ivug_medialist_get_data(pMainView->cur_mitem);
		currentindex =  pData->index;
	}

	if (pMainView->prev_mouse_point != 0 &&  currentindex != -1 && count != -1 && ivug_isslide_enabled(pMainView->pSliderNew) && pMainView->bmultitouchsliding == false) {
		if (pMainView->currentphotocam == PHOTOCAM_0) {
			evas_object_geometry_get(pMainView->photocam0, &bx, &by, &bw, &bh);
		} else if (pMainView->currentphotocam == PHOTOCAM_1) {
			evas_object_geometry_get(pMainView->photocam, &bx, &by, &bw, &bh);
		} else if (pMainView->currentphotocam == PHOTOCAM_2) {
			evas_object_geometry_get(pMainView->photocam2, &bx, &by, &bw, &bh);
		}

		MSG_MAIN_HIGH("current index = %d , diff %d", currentindex, ev->cur.output.x - pMainView->prev_mouse_point);
		if ((ev->cur.output.x - pMainView->prev_mouse_point < 0 && (currentindex + 1 < count
		        || (currentindex + 1 == count  && bx > 0))) || (ev->cur.output.x - pMainView->prev_mouse_point > 0
		                && (currentindex > 0))) {
			pMainView->is_moved = true;

			int diffX = pMainView->last_prev_mouse_point - ev->cur.output.x ;
			if (pMainView->is_play_Icon == true && (diffX > 10 || diffX < -10)) {
				pMainView->is_play_Icon = false;
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
			}

			ivug_slider_set_current_Photocam(pMainView->pSliderNew, pMainView->currentphotocam);
			ivug_slider_set_Photocam_moved(pMainView->pSliderNew, pMainView->is_moved);
			if (pMainView->currentphotocam == PHOTOCAM_0) {
				evas_object_geometry_get(pMainView->photocam0, &bx, &by, &bw, &bh);
				MSG_MAIN_HIGH("_on_slider_mouse_moved current x,y %d,%d", bx, by);
				evas_object_move(pMainView->photocam0, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				if (currentindex + 1 != count) {
					evas_object_geometry_get(pMainView->photocam, &bx, &by, &bw, &bh);
					evas_object_move(pMainView->photocam, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				}

				if (currentindex != 0) {
					evas_object_geometry_get(pMainView->photocam2, &bx, &by, &bw, &bh);
					evas_object_move(pMainView->photocam2, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				}
			} else if (pMainView->currentphotocam == PHOTOCAM_1) {
				evas_object_geometry_get(pMainView->photocam, &bx, &by, &bw, &bh);
				MSG_MAIN_HIGH("_on_slider_mouse_moved current x,y %d,%d", bx, by);
				evas_object_move(pMainView->photocam, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				if (currentindex + 1 != count) {
					evas_object_geometry_get(pMainView->photocam2, &bx, &by, &bw, &bh);
					evas_object_move(pMainView->photocam2, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				}

				if (currentindex != 0) {
					evas_object_geometry_get(pMainView->photocam0, &bx, &by, &bw, &bh);
					evas_object_move(pMainView->photocam0, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				}
			} else if (pMainView->currentphotocam == PHOTOCAM_2) {
				evas_object_geometry_get(pMainView->photocam2, &bx, &by, &bw, &bh);
				MSG_MAIN_HIGH("_on_slider_mouse_moved current x,y %d,%d", bx, by);
				evas_object_move(pMainView->photocam2, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				if (currentindex + 1 != count) {
					evas_object_geometry_get(pMainView->photocam0, &bx, &by, &bw, &bh);
					evas_object_move(pMainView->photocam0, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				}

				if (currentindex != 0) {
					evas_object_geometry_get(pMainView->photocam, &bx, &by, &bw, &bh);
					evas_object_move(pMainView->photocam, bx + (ev->cur.output.x - pMainView->prev_mouse_point) , by);
				}
			}
		}
		/*
		 * Below condition check if the last
		 * item in the folder is video and when
		 * slide left slightly video play icon disappear
		 * and again come back at original position
		 * then video play icon should appear
		 */
		Media_Data *pData = ivug_medialist_get_data(pMainView->cur_mitem);
		if ((pMainView->last_prev_mouse_point - ev->cur.output.x > -10) && (pMainView->is_play_Icon == false) && (currentindex + 1 == count) && (pData->slide_type == SLIDE_TYPE_VIDEO)) {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
			pMainView->is_play_Icon = true;
		}
	}

	pMainView->prev_mouse_point = ev->cur.output.x;
	//	pMainView->last_prev_mouse_point =  ev->prev.output.x;
}

void update_check(Ivug_MainView *pMainView)
{
	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	Eina_List *l = NULL;
	void *data = NULL;
	char **files = NULL;
	int i = 0 ;
	int check = 0;

	files = (char **)malloc(sizeof(char *) * pMainView->total_selected);
	if (!files) {
		MSG_MAIN_WARN("failed to allocate memory");
		return;
	}

	if (pMainView->selected_path_list) {
		EINA_LIST_FOREACH(pMainView->selected_path_list, l, data) {
			files[i] = strdup((char *)data);
			MSG_MAIN_HIGH("file is %s", files[i]);
			if (strcmp(mdata->filepath, files[i]) == 0) {
				evas_object_color_set(pMainView->check, 255, 255, 255, 255);
				elm_check_state_set(pMainView->check, EINA_TRUE);
				check = 1;
			}
			i++;
		}
		if (check == 0) {
			evas_object_color_set(pMainView->check, 128, 138, 137, 255);
			elm_check_state_set(pMainView->check, EINA_FALSE);
		}
	}

	i--;
	while(i >= 0) {
		free(files[i--]);
	}
	free(files);


	char buf[64] = {0,};
	snprintf(buf, 64, GET_STR(IDS_PD_SELECTED), pMainView->total_selected);
	elm_layout_text_set(pMainView->select_bar, "elm.text.title", buf);
}
/*Do the respective operation on releasing the finger from the screen*/
void _on_slider_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up *) event_info;
	MSG_MAIN_HIGH("mouse up (%d,%d)", ev->output.x, ev->output.y);
	if (pMainView->slide_state == true || ivug_isphotocam_reset(pMainView->pSliderNew)) {
		MSG_MAIN_HIGH("Sliding is happening");
		return ;//Do not accept the flick event  if transition is not completed
	}
	int count = -1;
	int currentindex = -1;
	pMainView->prevphotocam = pMainView->currentphotocam;

	if (pMainView->is_moved && ivug_isslide_enabled(pMainView->pSliderNew)) {

		if (pMainView->cur_mitem) {
			count = ivug_medialist_get_count(pMainView->mList);
			Media_Data *pData = ivug_medialist_get_data(pMainView->cur_mitem);
			currentindex =  pData->index;
			MSG_MAIN_HIGH(" _main_view_eventbox_flick_left_cb currentindex = %d count =%d ", currentindex, count);
		}

		int diffX = pMainView->last_prev_mouse_point - ev->output.x ;

		if (diffX > 10) {
			//left flick  code

			if (count != -1 && currentindex != -1 && currentindex + 1 < count) {
				if (pMainView->currentphotocam == PHOTOCAM_1) {
					pMainView->currentphotocam = PHOTOCAM_2;
				} else if (pMainView->currentphotocam == PHOTOCAM_2) {
					pMainView->currentphotocam = PHOTOCAM_0;
				} else if (pMainView->currentphotocam == PHOTOCAM_0) {
					pMainView->currentphotocam = PHOTOCAM_1;
				}

				MSG_MAIN_HIGH("prevphotocam = %d current photocam %d", pMainView->prevphotocam, pMainView->currentphotocam);
				pMainView->cur_mitem = ivug_medialist_get_next(pMainView->mList, pMainView->cur_mitem);
				ivug_medialist_set_current_item(pMainView->mList, pMainView->cur_mitem);

				// Update Main View.
				if (pMainView->bShowMenu == true) {
					_update_main_view(pMainView);
				}
				ivug_medialist_get_data(pMainView->cur_mitem);

				if (pMainView->slide_move_timer) {
					ecore_timer_del(pMainView->slide_move_timer);
					pMainView->slide_move_timer = NULL;
				}

				pMainView->slide_move_timer = ecore_timer_add(PC_MOVE_INTERVAL_TIME, _ivug_left_move_interval, pMainView);
				pMainView->slide_state = true;//set it to true until the tranition completes.
#ifdef USE_THUMBLIST
				if (pMainView->thumbs) {
					Media_Item *mItem = ivug_medialist_get_current_item(pMainView->mList);

					Image_Object *img = NULL;
					img = ivug_thumblist_find_item_by_data(pMainView->thumbs, mItem);		// Find thumb item of new image,.
					if (img != NULL) {
						pMainView->bSetThmByUser = true;
						ivug_thumblist_select_item(pMainView->thumbs, img);
					}
				}
#endif
			}
		} else if (diffX < (-10)) {
			// right flick code

			if (currentindex != -1 && currentindex > 0) {
				if (pMainView->currentphotocam == PHOTOCAM_1) {
					pMainView->currentphotocam = PHOTOCAM_0;
				} else if (pMainView->currentphotocam == PHOTOCAM_2) {
					pMainView->currentphotocam = PHOTOCAM_1;
				} else if (pMainView->currentphotocam == PHOTOCAM_0) {
					pMainView->currentphotocam = PHOTOCAM_2;
				}

				pMainView->cur_mitem = ivug_medialist_get_prev(pMainView->mList, pMainView->cur_mitem);
				ivug_medialist_set_current_item(pMainView->mList, pMainView->cur_mitem);

				// Update Main View.
				if (pMainView->bShowMenu == true) {
					_update_main_view(pMainView);
				}

				ivug_medialist_get_data(pMainView->cur_mitem);

				if (pMainView->slide_move_timer) {
					ecore_timer_del(pMainView->slide_move_timer);
					pMainView->slide_move_timer = NULL;
				}

				pMainView->slide_move_timer = ecore_timer_add(PC_MOVE_INTERVAL_TIME, _ivug_right_move_interval, pMainView);
				pMainView->slide_state = true;//set it to true until the tranition completes.
#ifdef USE_THUMBLIST
				if (pMainView->thumbs) {
					Media_Item *mItem = ivug_medialist_get_current_item(pMainView->mList);

					Image_Object *img = NULL;
					img = ivug_thumblist_find_item_by_data(pMainView->thumbs, mItem);		// Find thumb item of new image,.
					if (img != NULL) {
						pMainView->bSetThmByUser = true;
						ivug_thumblist_select_item(pMainView->thumbs, img);
					}
				}
#endif
			}
		} else {
			Media_Item *mItem = ivug_medialist_get_current_item(pMainView->mList);
			Media_Data *mdata = ivug_medialist_get_data(mItem);
			char *mime_type = ivug_fileinfo_get_mime_type(mdata->filepath);
			Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);
			if (mime_type && (strncmp(mime_type, "video/", strlen("video/"))) == 0) {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
				pMainView->is_play_Icon = true;
			} else {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
				pMainView->is_play_Icon = false;
			}
		}
	}
	pMainView->prev_mouse_point = 0;
	pMainView->last_prev_mouse_point = 0;
	pMainView->is_moved = false;
	return;
}

/* slide the image on flick left */
Eina_Bool _ivug_left_move_interval(void *data)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	bool movepossible = false;
	update_check(pMainView);
	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);

	ivug_disable_gesture(pMainView->pSliderNew);

	if (pMainView->prevphotocam == PHOTOCAM_1) {
		elm_photocam_zoom_mode_set(pMainView->photocam, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam, PC_POSITION_LEFT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam2, PC_POSITION_CENTER);
	} else if (pMainView->prevphotocam == PHOTOCAM_2) {
		elm_photocam_zoom_mode_set(pMainView->photocam2, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam2, PC_POSITION_LEFT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam0, PC_POSITION_CENTER);
	} else if (pMainView->prevphotocam == PHOTOCAM_0) {
		elm_photocam_zoom_mode_set(pMainView->photocam0, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam0, PC_POSITION_LEFT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam, PC_POSITION_CENTER);
	}

	if (!movepossible) {
		MSG_MAIN_HIGH("left_transit_done signal triggered");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "left_transit_done", "imageview_area");
		Media_Item *next_mitem = ivug_medialist_get_next(pMainView->mList, pMainView->cur_mitem);

		if (next_mitem) {
			Media_Data *pData = ivug_medialist_get_data(next_mitem);

			if (pData->slide_type == SLIDE_TYPE_IMAGE) {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
				pMainView->is_play_Icon = false;
			} else {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
				pMainView->is_play_Icon = true;
			}
		}
		pMainView->slide_move_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_RENEW;
}

/* slide the image on flick right */
Eina_Bool _ivug_right_move_interval(void *data)
{

	MSG_MAIN_HIGH("_ivug_right_move_interval");
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	bool movepossible = false;
	update_check(pMainView);
	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);

	ivug_disable_gesture(pMainView->pSliderNew);

	if (pMainView->prevphotocam == PHOTOCAM_1) {
		elm_photocam_zoom_mode_set(pMainView->photocam, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam, PC_POSITION_RIGHT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam0, PC_POSITION_CENTER);
	} else if (pMainView->prevphotocam == PHOTOCAM_2) {
		elm_photocam_zoom_mode_set(pMainView->photocam2, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam2, PC_POSITION_RIGHT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam, PC_POSITION_CENTER);
	} else if (pMainView->prevphotocam == PHOTOCAM_0) {
		elm_photocam_zoom_mode_set(pMainView->photocam0, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam0, PC_POSITION_RIGHT);
		movepossible = _main_view_object_move_(pMainView, pMainView->photocam2, PC_POSITION_CENTER);
	}

	if (!movepossible) {
		MSG_MAIN_HIGH("right_transit_done signal triggered");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "right_transit_done", "imageview_area");
		Media_Item *prev_mitem = ivug_medialist_get_prev(pMainView->mList, pMainView->cur_mitem);

		if (prev_mitem) {
			Media_Data *pmData = ivug_medialist_get_data(prev_mitem);

			if (pmData->slide_type == SLIDE_TYPE_IMAGE) {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
				pMainView->is_play_Icon = false;
			} else {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
				pMainView->is_play_Icon = true;
			}
		}

		pMainView->slide_move_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	return ECORE_CALLBACK_RENEW;
}

void ivug_update_favourite_button(Ivug_MainView *pMainView)
{
	char *default_thumbnail_edj_path = DEFAULT_THUMBNAIL_PATH;
	if ((pMainView->mode == IVUG_MODE_NORMAL || pMainView->mode == IVUG_MODE_CAMERA || pMainView->view_by == IVUG_VIEW_BY_FOLDER || pMainView->view_by == IVUG_VIEW_BY_ALL)
			&& pMainView->mode != IVUG_MODE_SETAS && pMainView->mode != IVUG_MODE_SELECT && pMainView->mode != IVUG_MODE_HIDDEN) {
		if (!strcmp(elm_photocam_file_get(ivug_slider_new_get_photocam(pMainView->pSliderNew)),
					default_thumbnail_edj_path)) {
			if (pMainView->btn_favorite) {
				evas_object_del(pMainView->btn_favorite);
				pMainView->btn_favorite = NULL;
			}
		} else {
			if (pMainView->btn_favorite == NULL) {
				pMainView->btn_favorite = create_favorite_button(pMainView->lyContent);
				evas_object_smart_callback_add(pMainView->btn_favorite, "clicked", _on_btn_favorite_cb, pMainView);
				elm_object_part_content_set(pMainView->lyContent, "elm.swallow.favorite", pMainView->btn_favorite);
			}
		}
	}
	free (default_thumbnail_edj_path);
}

/* Move the image until image fits the screen*/
bool _main_view_object_move_(Ivug_MainView *pMainView, Evas_Object *obj, int photocampos)
{
	int bx, by, bw, bh;
	evas_object_geometry_get(obj, &bx, &by, &bw, &bh);
	Evas_Object *win = gGetCurrentWindow();
	int wx, wy, ww, wh;
	evas_object_geometry_get(win, &wx, &wy, &ww, &wh);

	MSG_MAIN_LOW("_main_view_object_move_ enter,ww =%d wh = %d", ww, wh);
	int pixelmovx = 15;
	int pixelmovy = by;

	if (photocampos  == PC_POSITION_LEFT) { //left   -240
		int rem = ww - ABS(bx);
		int rempixels = rem % pixelmovx;
		int count = rem / pixelmovx;
		if (count > 0) {
			evas_object_move(obj, bx - pixelmovx, pixelmovy);
			evas_object_geometry_get(obj, &bx, &by, &bw, &bh);
		}
		evas_object_move(obj, bx - rempixels, pixelmovy);
		if (count  == 0) {
			return false;
		}
	} else if (photocampos == PC_POSITION_CENTER) { //centre   0
		int rem =  ABS(bx);
		int rempixels = rem % pixelmovx;
		int count = rem / pixelmovx;
		if (count > 0) {
			if (bx > 0) {
				evas_object_move(obj, bx - pixelmovx, pixelmovy);
			} else {
				evas_object_move(obj, bx + pixelmovx, pixelmovy);
			}
			evas_object_geometry_get(obj, &bx, &by, &bw, &bh);
		}
		if (bx > 0) {
			evas_object_move(obj, bx - rempixels, pixelmovy);
		} else {
			evas_object_move(obj, bx + rempixels, pixelmovy);
		}
		if (count  == 0) {
			return false;
		}

	} else if (photocampos == PC_POSITION_RIGHT) { //right   240
		int rem = ww - ABS(bx);
		int rempixels = rem % pixelmovx;
		int count = rem / pixelmovx;
		if (count > 0) {
			evas_object_move(obj, bx + pixelmovx, pixelmovy);
			evas_object_geometry_get(obj, &bx, &by, &bw, &bh);
		}
		evas_object_move(obj, bx + rempixels, pixelmovy);
		if (count  == 0) {
			return false;
		}
	}
	return true;
}

/* When the left transition is completed Rearrange the photocam images*/
void
_ivug_main_view_left_transit_by_item_complete_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	MSG_MAIN_HIGH("emission: %s, source: %s", emission, source);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);

	if (pMainView->prevphotocam == PHOTOCAM_1) {
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_stop", "imageview_area_temp2");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_left", "imageview_area");

	} else if (pMainView->prevphotocam == PHOTOCAM_2) {
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_stop", "imageview_area_temp0");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_left", "imageview_area_temp2");

	} else if (pMainView->prevphotocam == PHOTOCAM_0) {
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_stop", "imageview_area");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_left", "imageview_area_temp0");
	}

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	if (mitem) {
		Media_Data *mData = ivug_medialist_get_data(mitem);

		if (mData->slide_type == SLIDE_TYPE_IMAGE) {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
			ivug_enable_gesture(pMainView->pSliderNew);
			pMainView->is_play_Icon = false;
		} else {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
			ivug_disable_gesture(pMainView->pSliderNew);
			pMainView->is_play_Icon = true;
		}
	}

	if (pMainView->currentphotocam == PHOTOCAM_2) {
		MSG_MAIN_HIGH("TC currentphotocam == 2");
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam2);
		const char *prev_iva = "imageview_area";
		const char *next_iva = "imageview_area_temp0";
		ivug_set_prev_next_photocam_images(pMainView, &pMainView->photocam, &pMainView->photocam0, prev_iva, next_iva);
	}
	if (pMainView->currentphotocam == PHOTOCAM_0) {
		MSG_MAIN_HIGH("TR currentphotocam == 0");
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam0);
		const char *prev_iva = "imageview_area_temp2";
		const char *next_iva = "imageview_area";
		ivug_set_prev_next_photocam_images(pMainView, &pMainView->photocam2, &pMainView->photocam, prev_iva, next_iva);
	}
	if (pMainView->currentphotocam == PHOTOCAM_1) {
		MSG_MAIN_HIGH("TR currentphotocam == 1");
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam);
		const char *prev_iva = "imageview_area_temp0";
		const char *next_iva = "imageview_area_temp2";
		ivug_set_prev_next_photocam_images(pMainView, &pMainView->photocam0, &pMainView->photocam2, prev_iva, next_iva);
	}
	pMainView->slide_state = false;//Transition is completed.

	ivug_update_favourite_button(pMainView);
}

/* When the right transition is completed Rearrange the photocam images*/
void
_ivug_main_view_right_transit_by_item_complete_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	MSG_MAIN_HIGH("emission: %s, source: %s", emission, source);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);

	if (pMainView->prevphotocam == PHOTOCAM_1) {
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_stop", "imageview_area_temp0");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_right", "imageview_area");
	} else if (pMainView->prevphotocam == PHOTOCAM_2) {
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_stop", "imageview_area");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_right", "imageview_area_temp2");
	} else if (pMainView->prevphotocam == PHOTOCAM_0) {
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_stop", "imageview_area_temp2");
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_right", "imageview_area_temp0");
	}

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	if (mitem) {
		Media_Data *mData = ivug_medialist_get_data(mitem);

		if (mData->slide_type == SLIDE_TYPE_IMAGE) {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
			ivug_enable_gesture(pMainView->pSliderNew);
			pMainView->is_play_Icon = false;
		} else {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
			ivug_disable_gesture(pMainView->pSliderNew);
			pMainView->is_play_Icon = true;
		}
	}

	if (pMainView->currentphotocam == PHOTOCAM_2) {
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam2);
		const char *prev_iva = "imageview_area";
		const char *next_iva = "imageview_area_temp0";
		ivug_set_prev_next_photocam_images(pMainView, &pMainView->photocam, &pMainView->photocam0, prev_iva, next_iva);
	}
	if (pMainView->currentphotocam == PHOTOCAM_0) {
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam0);
		const char *prev_iva = "imageview_area_temp2";
		const char *next_iva = "imageview_area";
		ivug_set_prev_next_photocam_images(pMainView, &pMainView->photocam2, &pMainView->photocam, prev_iva, next_iva);
	}
	if (pMainView->currentphotocam == PHOTOCAM_1) {
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam);
		const char *prev_iva = "imageview_area_temp0";
		const char *next_iva = "imageview_area_temp2";
		ivug_set_prev_next_photocam_images(pMainView, &pMainView->photocam0, &pMainView->photocam2, prev_iva, next_iva);
	}
	pMainView->slide_state = false;//Transition is completed.

	ivug_update_favourite_button(pMainView);
}

/* Used to create a new photocam image*/
void ivug_create_new_photocam_image(void *data, Evas_Object **cur_pc, const char *cur_iva)
{
	MSG_MAIN_HIGH("ivug_create_new_photocam_image");
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);
	*cur_pc = elm_photocam_add(sn_layout);
	elm_photocam_gesture_enabled_set(*cur_pc, EINA_TRUE);
	evas_object_size_hint_expand_set(*cur_pc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(*cur_pc, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(sn_layout, cur_iva, *cur_pc);
	elm_photocam_zoom_mode_set(*cur_pc, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
	elm_photocam_paused_set(*cur_pc, EINA_TRUE);
	evas_object_size_hint_weight_set(*cur_pc, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);

}

/*Make the  previous and next photocam images ready for smooth movement of images*/
void ivug_set_prev_next_photocam_images(void *data, Evas_Object **prev_pc, Evas_Object **next_pc, const char *prev_iva, const char *next_iva)
{
	MSG_MAIN_HIGH("ivug_main_view_set_prev_next_photocam_images");
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);
	Evas_Load_Error err = EVAS_LOAD_ERROR_NONE;

	Media_Item *prev_mitem = ivug_medialist_get_prev(pMainView->mList, pMainView->cur_mitem);

	char *edj_file = DEFAULT_THUMBNAIL_PATH;
	// Update Main View.
	if (pMainView->bShowMenu == true) {
		_update_main_view(pMainView);
	}
	if (prev_mitem) {
		Media_Data *pmData = ivug_medialist_get_data(prev_mitem);
		if ((*prev_pc) == NULL) {
			ivug_create_new_photocam_image(pMainView, prev_pc, prev_iva);
		}

		if (pmData->slide_type == SLIDE_TYPE_VIDEO) {
			elm_photocam_file_set(*prev_pc, pmData->thumbnail_path);
		} else {
			err = elm_photocam_file_set(*prev_pc, pmData->filepath);

			if (EVAS_LOAD_ERROR_NONE != err) {
				MSG_HIGH("Loading default Thumbnail");
				elm_photocam_file_set(*prev_pc, edj_file);
			}
		}
		evas_object_show(*prev_pc);
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_left", prev_iva);
	}

	Media_Item *next_mitem = ivug_medialist_get_next(pMainView->mList, pMainView->cur_mitem);

	// Update Main View.
	if (pMainView->bShowMenu == true) {
		_update_main_view(pMainView);
	}
	if (next_mitem) {
		Media_Data *pmData = ivug_medialist_get_data(next_mitem);
		if ((*next_pc) == NULL) {
			ivug_create_new_photocam_image(pMainView, next_pc, next_iva);
		}

		if (pmData->slide_type == SLIDE_TYPE_VIDEO) {
			elm_photocam_file_set(*next_pc, pmData->thumbnail_path);
		} else {
			err = elm_photocam_file_set(*next_pc, pmData->filepath);

			if (EVAS_LOAD_ERROR_NONE != err) {
				MSG_HIGH("Loading default Thumbnail");
				elm_photocam_file_set(*next_pc, edj_file);
			}
		}
		evas_object_show(*next_pc);
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_right", next_iva);
	}
	free(edj_file);
}
