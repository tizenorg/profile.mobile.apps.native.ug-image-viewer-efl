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

#include <Evas.h>
#include <Ecore.h>
#include <Elementary.h>
#include <math.h>
#include <Eina.h>
#include <app.h>

#include "ivug-debug.h"
#include "ivug-util.h"

#include "ivug-media.h"

#include "ivug-thumblist.h"

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-THUMB"


static inline char * ivug_get_resource_path() {
	char * path = app_get_resource_path();
	return path;
}

static inline char* full_path(const char *str1, const char *str2) {
	char path[1024] = {};
	snprintf(path, 1024, "%s%s", str1, str2);
	char *full_path = strdup(path);
	return full_path;
}


#define UG_RES_PATH 		ivug_get_resource_path()
#define EDJ_PATH				full_path(UG_RES_PATH, "edje")
#define IMAGE_PATH						full_path(UG_RES_PATH, "edje/images")


#define DEFAULT_THUMBNAIL_PATH		full_path(IMAGE_PATH,"/T01_Nocontents_broken.png")

#define EDJ_THUMBLIST 	full_path(EDJ_PATH,"/ivug-thumblist.edj")
#define EDJ_ICON 		full_path(EDJ_PATH,"/ivug-icons.edj")


//#undef TEST_EDIT_MODE
#define TEST_EDIT_MODE

/*
	Currently gengrid does not have feature to display focused item as highlighted.
	So focely emit signal so as to dislay highlight
*/
#define FEATURE_HIGHLIGHT

typedef enum {
	IV_THUMBLIST_MODE_LIST,
	IV_THUMBLIST_MODE_BESTPIC,
	IV_THUMBLIST_MODE_EDIT,
} ivug_thumblist_mode;


typedef struct _Thumbnail Thumbnail;

typedef struct {
	Evas_Object *gengrid;

	ivug_thumblist_mode eGridMode;
	Thumbnail *eSelected;			// Selected item in edit mode

	Elm_Gengrid_Item_Class *gic;
} ThumbList;

struct _Thumbnail {
/*
	Represent each item
*/
	ThumbList *thmlist;		// Parent Object.

	Evas_Object *content;

	Elm_Object_Item *item;

	bool bRealized;
	bool bChecked;			// Edit Mode.
	bool bHightlighted;

	Thumblist_Metaphore tMetaphor;		// Metaphor

	bool bVideo;

	char *path;
	void *data;
} ;

#define IV_THUMBLIST(obj) \
	static_cast<ThumbList *>(evas_object_data_get((obj), "CMyThumbList"))

#define THUMB_SIZE (45)

void _image_preloaded(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	evas_object_show(o);
}

static Evas_Object *create_contents(Evas_Object *parent, const char *path)
{
	Evas_Object *image = evas_object_image_add(evas_object_evas_get(parent));
	evas_object_event_callback_add(image, EVAS_CALLBACK_IMAGE_PRELOADED, _image_preloaded, NULL);
	evas_object_image_alpha_set(image, EINA_FALSE);

	evas_object_image_scale_hint_set(image, EVAS_IMAGE_SCALE_HINT_STATIC);
	evas_object_image_smooth_scale_set(image, EINA_TRUE);

//	evas_object_image_load_scale_down_set(image, 2);
	if (path == NULL) {
		MSG_WARN("thumbnail is not loaded yet");
		return image;
	}

	evas_object_image_file_set(image, path, NULL);

	MSG_LOW("Load image : %s", ivug_get_filename(path));
	Evas_Load_Error err = evas_object_image_load_error_get(image);

	if (EVAS_LOAD_ERROR_NONE != err) {
		MSG_ERROR("Cannot load file : Err(%d) %s", err, path);
		// Load default.
		char *edj_path = DEFAULT_THUMBNAIL_PATH;
		evas_object_image_file_set(image, edj_path, NULL);
		free(edj_path);

		Evas_Load_Error err = evas_object_image_load_error_get(image);

		if (EVAS_LOAD_ERROR_NONE != err) {
			MSG_FATAL("Cannot load default image");
			return NULL;
		}
	}

	// Preserve aspect ratio.
	int imgW, imgH;
	evas_object_image_size_get(image, &imgW, &imgH);

	MSG_LOW("Img Size : WH(%dx%d)", imgW, imgH);

	if (imgW != 0 && imgH != 0) {
		int fX, fY, fW, fH;
		if (imgW < imgH) {
			fW = THUMB_SIZE;
			fH = fW * imgH / imgW;

			fX = 0;
			fY = (THUMB_SIZE - fH) / 2;
		} else {
			fH = THUMB_SIZE;
			fW = fH * imgW / imgH;

			fY = 0;
			fX = (THUMB_SIZE - fW) / 2;
		}

		MSG_LOW("Img Fill : XWWH(%d,%d,%d,%d)", fX, fY, fW, fH);
// Center align
		evas_object_image_load_size_set(image, fW, fH);
		evas_object_image_fill_set(image, fX, fY, fW, fH);
	}

	evas_object_image_preload(image, EINA_FALSE /* Eina_Bool cancel */); // If cancel is	true set, it will remove the image from the workqueue

	return image;

}


static Evas_Object *create_edje_content(Evas_Object *parent, const char *path)
{
	Evas_Object *layout;

	layout = elm_layout_add(parent);

	char *edj_path = EDJ_THUMBLIST;
	if (elm_layout_file_set(layout, edj_path, "thumblist.item") == EINA_FALSE) {
		MSG_ERROR("Cannot load thumlist edj");
		free(edj_path);
		return NULL;
	}
	free(edj_path);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(layout);
	Evas_Object *image = create_contents(layout, path);

	if (image == NULL) {
		MSG_FATAL("Cannot create thumb item");
	}

	elm_object_part_content_set(layout, "item", image);

	return layout;
}

static void _set_highlighted(Thumbnail *thm)
{
	if (thm->content) {	//before contents callback it will be null
		edje_object_signal_emit(elm_layout_edje_get(thm->content), "elm,state,selected", "app");
	}

}

static void
_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Thumbnail *thm = (Thumbnail *)data;

	thm->bChecked = !elm_check_state_get(obj);

	MSG_MED("Check box Changed! newState=%d", thm->bChecked);
	elm_check_state_set(obj, thm->bChecked);

//	elm_gengrid_item_selected_set(thm->item, thm->bChecked);
}

static Evas_Object *
grid_content_get(void *data, Evas_Object *obj, const char *part)
{
	Thumbnail *thm = (Thumbnail *)data;
// TODO : For bestshot, use edje.
	MSG_LOW("Content get : %s", part);
	char *edj_path = EDJ_ICON;

	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *contents = create_edje_content(obj, (thm->path));

		thm->content = contents;

		Evas_Object *icon = NULL;

		switch (thm->tMetaphor) {
		case THUMBLIST_NONE:
			edje_object_signal_emit(elm_layout_edje_get(contents), "thumbnail,hide,metaphore", "elm");
			break;

		case THUMBLIST_SOUNDPIC:
			icon = elm_icon_add(contents);
			elm_image_no_scale_set(icon, EINA_TRUE);
			elm_image_aspect_fixed_set(icon, EINA_TRUE);

			if (elm_image_file_set(icon, edj_path, "icon.sound_scene") == EINA_FALSE) {
				MSG_ERROR("Cannot load EDJ_ICON");
			}

			elm_layout_content_set(contents, "thumblist.metaphore", icon);

			edje_object_signal_emit(elm_layout_edje_get(contents), "thumbnail,show,metaphore", "elm");
			break;


		case THUMBLIST_PANORAMA:
			icon = elm_icon_add(contents);
			elm_image_no_scale_set(icon, EINA_TRUE);
			elm_image_aspect_fixed_set(icon, EINA_TRUE);

			if (elm_image_file_set(icon, edj_path, "icon.panorama") == EINA_FALSE) {
				MSG_ERROR("Cannot load EDJ_ICON");
			}

			elm_layout_content_set(contents, "thumblist.metaphore", icon);

			edje_object_signal_emit(elm_layout_edje_get(contents), "thumbnail,show,metaphore", "elm");
			break;


		case THUMBLIST_BURSTSHOT:
			icon = elm_icon_add(contents);
			elm_image_no_scale_set(icon, EINA_TRUE);
			elm_image_aspect_fixed_set(icon, EINA_TRUE);

			if (elm_image_file_set(icon, edj_path, "icon.burst") == EINA_FALSE) {
				MSG_ERROR("Cannot load EDJ_ICON");
			}

			elm_layout_content_set(contents, "thumblist.metaphore", icon);

			edje_object_signal_emit(elm_layout_edje_get(contents), "thumbnail,show,metaphore", "elm");
			break;

		default:
			MSG_ERROR("Unknwon metaphore(%d)", thm->tMetaphor);
		}

		if (thm->bVideo == true) {
			icon = elm_icon_add(contents);
			elm_image_no_scale_set(icon, EINA_TRUE);
			elm_image_aspect_fixed_set(icon, EINA_TRUE);

			if (elm_image_file_set(icon, edj_path, "btn.video.play.80.80") == EINA_FALSE) {
				MSG_ERROR("Cannot load EDJ_ICON");
			}

			elm_layout_content_set(contents, "thumblist.vicon", icon);

			edje_object_signal_emit(elm_layout_edje_get(contents), "thumbnail,show,vicon", "elm");
		} else {
			edje_object_signal_emit(elm_layout_edje_get(contents), "thumbnail,hide,vicon", "elm");
		}

		if (thm->thmlist->eGridMode == IV_THUMBLIST_MODE_EDIT) {
			if (thm->thmlist->eSelected == thm) {
				_set_highlighted(thm->thmlist->eSelected);
			}
		} else {
#ifdef FEATURE_HIGHLIGHT
			if (elm_gengrid_item_selected_get(thm->item) == EINA_TRUE) {

				edje_object_signal_emit(elm_layout_edje_get(thm->content), "elm,state,selected", "app");
			}
#endif
		}

		free(edj_path);

		return contents;
	} else if (thm->thmlist->eGridMode == IV_THUMBLIST_MODE_EDIT && !strcmp(part, "elm.swallow.end")) {
		MSG_MED("Checked ! %d", thm->bChecked);

		Evas_Object *ck = elm_check_add(obj);
		elm_object_style_set(ck, "grid");

		evas_object_propagate_events_set(ck, EINA_FALSE);
		elm_check_state_set(ck, thm->bChecked);

		if (!elm_config_access_get()) {
			evas_object_smart_callback_add(ck, "changed", _item_check_changed_cb, data);
		} else {
			evas_object_repeat_events_set(ck, EINA_TRUE);
			elm_access_object_unregister(ck);
		}

		free(edj_path);
		return ck;
	}

	free(edj_path);
	return NULL;
}

static void grid_content_del(void *data, Evas_Object *obj)
{
	Thumbnail *thm = (Thumbnail *)data;

	if (thm->thmlist->eSelected == thm) {
		thm->thmlist->eSelected = NULL;
	}

	free(thm->path);
	free(thm);
}


static void
grid_longpress(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_ERROR("Long pressed!. %s", (thm->path));

	if (thm->thmlist->eGridMode == IV_THUMBLIST_MODE_LIST) {
#ifdef TEST_EDIT_MODE
// Change to Edit mode.
		ivug_thumblist_set_edit_mode(obj, TRUE);		// Set Edit Mode

// Update gengrid
		elm_gengrid_realized_items_update(thm->thmlist->gengrid);
#endif
		//evas_object_smart_callback_call(thm->thmlist->gengrid, "changed,mode", (void *)"edit");
		return;
	} else if (thm->thmlist->eGridMode == IV_THUMBLIST_MODE_EDIT) {
		MSG_ERROR("Long press in Edit Mode");

		Evas_Object *check;

// Toggle check box
//		elm_gengrid_item_selected_set(thm->item, EINA_FALSE);
		check = elm_object_item_part_content_get(thm->item, "elm.swallow.end");
		if (!check) {
			return;
		}

		thm->bChecked = !elm_check_state_get(check);
		elm_check_state_set(check, thm->bChecked);

		return;
	}

	if (thm->tMetaphor == THUMBLIST_BESTPIC) {
		thm->tMetaphor = THUMBLIST_NONE;
		elm_gengrid_item_update(item);

		evas_object_smart_callback_call(thm->thmlist->gengrid, "set,bestpic", item);

		// Change to selected item.
		elm_gengrid_item_selected_set(thm->item, EINA_TRUE);
	} else {
		thm->tMetaphor = THUMBLIST_BESTPIC;
		elm_gengrid_item_update(item);

		evas_object_smart_callback_call(thm->thmlist->gengrid, "unset,bestpic", item);
		elm_gengrid_item_selected_set(thm->item, EINA_TRUE);
	}


}

static void _on_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	ThumbList *thmlist = (ThumbList *)data;
	MSG_ASSERT(thmlist != NULL);

	MSG_MED("Thumblist removing");

	elm_gengrid_item_class_free(thmlist->gic);
	free(thmlist);

}

static void _on_resized(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	if (w == 0  || h == 0) {
		return;
	}

	MSG_MED("Thumblist resized resized geomtery XYWH(%d,%d,%d,%d)", x, y, w, h);
	/*
		double scale = elm_config_scale_get();
		int w = (int)((8*scale)+(162*scale)+(8*scale)); // width L + W + R
		int h = (int)((9*scale)+(162*scale)+(9*scale)); // height U + H + D
	*/
//	elm_gengrid_item_size_set(thmlist->gengrid, h * elm_config_scale_get(), h * elm_config_scale_get());

	Elm_Object_Item *item = elm_gengrid_selected_item_get(obj);

	if (item != NULL) {
		elm_gengrid_item_update(item);
		Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

#ifdef FEATURE_HIGHLIGHT
		if (thm->content != NULL) {
			edje_object_signal_emit(elm_layout_edje_get(thm->content), "elm,state,selected", "app");
		}
#endif
		elm_gengrid_item_show(item, ELM_GENGRID_ITEM_SCROLLTO_IN);
	}

}


void _on_item_selected(void *data, Evas_Object *obj, void *event_info)
{
	Thumbnail *thm = (Thumbnail *)data;

	MSG_SEC("Item selected. %s", ivug_get_filename((thm->path)));

	if (thm->thmlist->eGridMode == IV_THUMBLIST_MODE_EDIT) {
		Evas_Object *check;

// Toggle check box
		check = elm_object_item_part_content_get(thm->item, "elm.swallow.end");
		if (!check) {
			return;
		}

		thm->bChecked = !elm_check_state_get(check);
		elm_check_state_set(check, thm->bChecked);

		MSG_MED("Grid Mode is Edit. Check State=%d", thm->bChecked);

		evas_object_smart_callback_call(thm->thmlist->gengrid, "item,edit,selected", thm->item);

		return ;
	}

#ifdef FEATURE_HIGHLIGHT
	if (thm->content) {
		edje_object_signal_emit(elm_layout_edje_get(thm->content), "elm,state,selected", "app");
	} else {
		MSG_ERROR("Contents is NULL. in selected callback");
	}
#endif

}


void _on_item_unselected(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_SEC("Item Unselected. %s thm->content 0x%08x", ivug_get_filename((thm->path)), thm->content);

	if (thm->thmlist->eGridMode == IV_THUMBLIST_MODE_EDIT) {
// Toggle check box
		return;
	}

#ifdef FEATURE_HIGHLIGHT
	if (thm->content) {
		edje_object_signal_emit(elm_layout_edje_get(thm->content), "elm,state,unselected", "app");
	} else {
		MSG_ERROR("Contents is NULL. in unselected callback");
	}
#endif

}


void _on_item_realized(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_LOW("Item Realized. %s", ivug_get_filename((thm->path)));

	thm->bRealized = true;
#ifdef FEATURE_HIGHLIGHT
	if (elm_gengrid_item_selected_get(item) == EINA_TRUE) {
		edje_object_signal_emit(elm_layout_edje_get(thm->content), "elm,state,selected", "app");
	}
#endif
	elm_gengrid_item_update(item);

	evas_object_smart_callback_call(thm->thmlist->gengrid, "item,realized", thm->data);
}

void _on_item_unrealized(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_LOW("Item Unrealized. %s", ivug_get_filename((thm->path)));

	thm->bRealized = false;

	thm->content = NULL;
}


void _on_item_changed(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	if (thm->thmlist->eGridMode != IV_THUMBLIST_MODE_EDIT) {
		MSG_MED("Selected from gengrid. thm=0x%08x", thm);
		evas_object_smart_callback_call(thm->thmlist->gengrid, "item,selected", item);
	}
}


static Thumbnail *_alloc_thumb(ThumbList *thmlist)
{
	Thumbnail *thm;

	thm = (Thumbnail *)calloc(1, sizeof(Thumbnail));
	IV_ASSERT(thm != NULL);

	thm->thmlist = thmlist;
	thm->bRealized = false;
	thm->tMetaphor = THUMBLIST_BESTPIC;

	return thm;
}

Evas_Object *ivug_thumblist_add(Evas_Object *parent)
{
	ThumbList *thmlist = (ThumbList *)calloc(1, sizeof(ThumbList));
	MSG_ASSERT(thmlist != NULL);

	Evas_Object *gengrid;

	gengrid = elm_gengrid_add(parent);

	elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);

	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_gengrid_align_set(gengrid, 0.5, 0.0);

	int iw, ih;
	iw = THUMB_SIZE * elm_config_scale_get();
	ih = THUMB_SIZE * elm_config_scale_get();

	elm_gengrid_item_size_set(gengrid, iw, ih);

	elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
	elm_scroller_bounce_set(gengrid, EINA_FALSE, EINA_FALSE);
	elm_gengrid_multi_select_set(gengrid, EINA_FALSE);

	evas_object_smart_callback_add(gengrid, "longpressed", grid_longpress, (void *)thmlist);
	evas_object_smart_callback_add(gengrid, "unselected", _on_item_unselected, (void *)thmlist);
	evas_object_smart_callback_add(gengrid, "realized", _on_item_realized, (void *)thmlist);
	evas_object_smart_callback_add(gengrid, "unrealized", _on_item_unrealized, (void *)thmlist);
	evas_object_smart_callback_add(gengrid, "selected", _on_item_changed, (void *)thmlist);

	evas_object_event_callback_add(gengrid, EVAS_CALLBACK_RESIZE, _on_resized, (void *)thmlist);
	evas_object_event_callback_add(gengrid, EVAS_CALLBACK_DEL, _on_del, (void *)thmlist);

	thmlist->eGridMode = IV_THUMBLIST_MODE_LIST;

	thmlist->gic = elm_gengrid_item_class_new();

	if (thmlist->gic != NULL) {
		thmlist->gic->item_style = "thumbgrid";
		thmlist->gic->func.text_get = NULL;
		thmlist->gic->func.content_get = grid_content_get;
		thmlist->gic->func.state_get = NULL;
		thmlist->gic->func.del = grid_content_del;
	}

	thmlist->gengrid = gengrid;

	evas_object_data_set(gengrid, "CMyThumbList", thmlist);

	MSG_MED("Created GenGrid. Obj=0x%08x", gengrid);
	return gengrid;

}

void
ivug_thumblist_set_bestpic_mode(Evas_Object *obj, Eina_Bool bBestPicMode)
{
	ThumbList *thmlist = IV_THUMBLIST(obj);
	MSG_ASSERT(thmlist != NULL);

	if (bBestPicMode == EINA_TRUE) {
		thmlist->eGridMode = IV_THUMBLIST_MODE_BESTPIC;
	} else {
		thmlist->eGridMode = IV_THUMBLIST_MODE_LIST;
	}
}


void
ivug_thumblist_set_edit_mode(Evas_Object *obj, Eina_Bool bEditMode)
{
	ThumbList *thmlist = IV_THUMBLIST(obj);
	MSG_ASSERT(thmlist != NULL);

	if (bEditMode == EINA_TRUE) {
		Elm_Object_Item *iter;
		Thumbnail *thm;

		iter = elm_gengrid_first_item_get(obj);

		while (iter != NULL) {
			thm = (Thumbnail *)elm_object_item_data_get(iter);

			thm->bChecked = false;
			iter = elm_gengrid_item_next_get(iter);
		}

		thmlist->eGridMode = IV_THUMBLIST_MODE_EDIT;
		elm_gengrid_select_mode_set(thmlist->gengrid, ELM_OBJECT_SELECT_MODE_ALWAYS);

		Elm_Object_Item *item = elm_gengrid_selected_item_get(obj);

		thm = (Thumbnail *)elm_object_item_data_get(item);
		thmlist->eSelected = thm;		// Store currented selected

		evas_object_smart_callback_call(thmlist->gengrid, "changed,mode", (void *)"edit");
	} else {
		if (thmlist->eSelected) {	// Highlighted item
			elm_gengrid_item_selected_set(thmlist->eSelected->item, EINA_TRUE);
			elm_gengrid_item_show(thmlist->eSelected->item, ELM_GENGRID_ITEM_SCROLLTO_IN);
		}

		thmlist->eGridMode = IV_THUMBLIST_MODE_LIST;
		elm_gengrid_select_mode_set(thmlist->gengrid, ELM_OBJECT_SELECT_MODE_DEFAULT);

		// Update gengrid
		elm_gengrid_realized_items_update(thmlist->gengrid);

		thmlist->eSelected = NULL;

		evas_object_smart_callback_call(thmlist->gengrid, "changed,mode", (void *)"list");

	}
}

Eina_Bool ivug_thumblist_get_edit_mode(Evas_Object *obj)
{
	ThumbList *thmlist = IV_THUMBLIST(obj);
	MSG_ASSERT(thmlist != NULL);

	return thmlist->eGridMode == IV_THUMBLIST_MODE_EDIT ? EINA_TRUE : EINA_FALSE;
}



Image_Object *ivug_thumblist_append_item(Evas_Object *obj, const char *thumbnail_path, bool bVideo, void *item_data)
{
	ThumbList *thmlist = IV_THUMBLIST(obj);
	MSG_ASSERT(thmlist != NULL);

	Thumbnail *thm = _alloc_thumb(thmlist);

	if (thumbnail_path == NULL) {
		thm->path = DEFAULT_THUMBNAIL_PATH;
	} else {
		thm->path = strdup(thumbnail_path);
	}
	thm->data = item_data;
	thm->tMetaphor = THUMBLIST_NONE;
	thm->bVideo = bVideo;

	thm->item = elm_gengrid_item_append(thmlist->gengrid, thmlist->gic, thm, _on_item_selected, thm);

	MSG_LOW("Append: %s Data=0x%08x", ivug_get_filename(thumbnail_path), item_data);

	return (Image_Object *)thm->item;
}


Image_Object *ivug_thumblist_prepend_item(Evas_Object *obj, const char *thumbnail_path, bool bVideo, void *item_data)
{
	ThumbList *thmlist = IV_THUMBLIST(obj);
	MSG_ASSERT(thmlist != NULL);

	Thumbnail *thm = _alloc_thumb(thmlist);

	if (thumbnail_path == NULL) {
		thm->path = DEFAULT_THUMBNAIL_PATH;
	} else {
		thm->path = strdup(thumbnail_path);
	}
	thm->data = item_data;
	thm->tMetaphor = THUMBLIST_NONE;
	thm->bVideo = bVideo;

	thm->item = elm_gengrid_item_prepend(thmlist->gengrid, thmlist->gic, thm, _on_item_selected, thm);

	MSG_LOW("Prepend: %s Data=0x%08x", ivug_get_filename(thumbnail_path), item_data);

	return (Image_Object *)thm->item;
}

unsigned int ivug_thumblist_items_count(Evas_Object *obj)
{
	ThumbList *thmlist = IV_THUMBLIST(obj);
	MSG_ASSERT(thmlist != NULL);

	return elm_gengrid_items_count(thmlist->gengrid);
}



Eina_Bool
ivug_thumblist_update_item(Evas_Object* obj, Image_Object *img, const char *thumbnail_path, bool bVideo, void *item_data)
{
	Elm_Object_Item *item = (Elm_Object_Item *)img;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	free(thm->path);
	if (thumbnail_path == NULL) {
		thm->path = DEFAULT_THUMBNAIL_PATH;
	} else {
		thm->path = strdup(thumbnail_path);
	}
	thm->data = item_data;
	thm->tMetaphor = THUMBLIST_NONE;
	thm->bVideo = bVideo;

	elm_gengrid_item_update(thm->item);
	return EINA_TRUE;
}


void ivug_thumblist_delete_item(Evas_Object *obj, Image_Object *img)
{

	Elm_Object_Item *item = (Elm_Object_Item *)img;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_HIGH("Delete Item: %s", (thm->path));

	elm_object_item_del(thm->item);
	thm->item = NULL;

// 	thm will be freed in grid_content_del()
}

void ivug_thumblist_clear_items(Evas_Object *obj)
{
	ThumbList *thmlist = IV_THUMBLIST(obj);
	MSG_ASSERT(thmlist != NULL);

// Remove all items
	elm_gengrid_clear(thmlist->gengrid);
}

void
ivug_thumblist_select_item(Evas_Object *obj, Image_Object *img)
{
	Elm_Object_Item *item = (Elm_Object_Item *)img;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	if (thm->thmlist->eGridMode != IV_THUMBLIST_MODE_EDIT) {
		if (ivug_thumblist_get_selected_item(obj) == img) {
			MSG_WARN("Same center.");
			return;
		}
	}

// How can realize wanted item??????????.
//	elm_gengrid_item_update(thm->item);

	bool bRealized = false;

	if (thm->bRealized == true) {
		bRealized = true;
	}

#if 1   // BRING_IN is not working..
	if (bRealized == true) {	// If distance is near(??).
		// Animated scroll to item.
		elm_gengrid_item_bring_in(thm->item, ELM_GENGRID_ITEM_SCROLLTO_IN);
	} else
#endif
	{
		// Jump and show item/.
		elm_gengrid_item_show(thm->item, ELM_GENGRID_ITEM_SCROLLTO_IN);
	}

	if (thm->thmlist->eGridMode == IV_THUMBLIST_MODE_EDIT) {
		MSG_MED("Selected item in Edit mode");

// UnHighlight Old.
#if 1
		Thumbnail *old = thm->thmlist->eSelected;

		if (old != NULL) {
			elm_gengrid_item_selected_set(old->item, EINA_FALSE);		// Item can be selected continously. so we set as unselected

			if (old->content) {
				edje_object_signal_emit(elm_layout_edje_get(old->content), "elm,state,unselected", "app");
			} else {
				MSG_ERROR("Contents is NULL. in unselected callback");
			}

		}

#endif
//		elm_gengrid_item_selected_set(thm->item, EINA_TRUE);
//		elm_gengrid_item_select_mode_set(thm->item, ELM_OBJECT_SELECT_MODE_ALWAYS);

// User selected externally
		_set_highlighted(thm);

		thm->thmlist->eSelected = thm;
		return ;
	}

	elm_gengrid_item_selected_set(thm->item, EINA_TRUE);

#ifdef FEATURE_HIGHLIGHT
	if (thm->content) {	//before contents callback it will be null
		edje_object_signal_emit(elm_layout_edje_get(thm->content), "elm,state,selected", "app");
	}
#endif

	MSG_HIGH("Set Center: %s", (thm->path));
}

Image_Object *ivug_thumblist_get_selected_item(Evas_Object* obj)
{
	Elm_Object_Item *item = elm_gengrid_selected_item_get(obj);

	return (Image_Object *)item;

}

void *ivug_thumblist_get_item_data(Evas_Object* obj, Image_Object *img)
{
	Elm_Object_Item *item = (Elm_Object_Item *)img;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_MED("GetItemData() thm=0x%08x", thm);

	return thm->data;
}

Thumblist_Metaphore
ivug_thumblist_get_item_mode(Evas_Object* obj, Image_Object *img)
{
	Elm_Object_Item *item = (Elm_Object_Item *)img;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_MED("GetItemBestpic() thm=0x%08x", thm);

	return thm->tMetaphor;
}

void
ivug_thumblist_set_item_mode(Evas_Object* obj, Image_Object *img, Thumblist_Metaphore mode)
{
	Elm_Object_Item *item = (Elm_Object_Item *)img;

	Thumbnail *thm = (Thumbnail *)elm_object_item_data_get(item);

	MSG_MED("SetItemBestpic() thm=0x%08x Mode=%d", thm, mode);

	thm->tMetaphor = mode;

	elm_gengrid_item_update(thm->item);
}



Image_Object *
ivug_thumblist_find_item_by_data(Evas_Object* obj, void *item_data)
{
// TODO... Need time checking

	Elm_Object_Item *iter;
	Thumbnail *thm;

	iter = elm_gengrid_first_item_get(obj);

	while (iter != NULL) {
		thm = (Thumbnail *)elm_object_item_data_get(iter);

		if (thm->data == item_data) {
			MSG_SEC("Found Item: %s Data=0x%08x", ivug_get_filename((thm->path)), item_data);
			return (Image_Object *)iter;
		}

		iter = elm_gengrid_item_next_get(iter);
	}

	MSG_WARN("No item founded. Data=0x%08x TotalCount=%d", item_data, elm_gengrid_items_count(obj));
	return NULL;
}


Eina_List/* Image_Object */ *
ivug_thumblist_get_items_bestpic(Evas_Object* obj)
{
	Eina_List *result = NULL;

	Elm_Object_Item *iter;
	Thumbnail *thm;

	iter = elm_gengrid_first_item_get(obj);

	while (iter != NULL) {
		thm = (Thumbnail *)elm_object_item_data_get(iter);

		if (thm->tMetaphor == THUMBLIST_BESTPIC) {
			MSG_SEC("Found Best pic Item: %s", ivug_get_filename((thm->path)));
			result = eina_list_append(result, iter);
		}

		iter = elm_gengrid_item_next_get(iter);
	}

	return result;

}


Eina_List/* Image_Object */ *
ivug_thumblist_get_items_checked(Evas_Object* obj)
{
	Eina_List *result = NULL;

	Elm_Object_Item *iter;
	Thumbnail *thm;

	iter = elm_gengrid_first_item_get(obj);

	while (iter != NULL) {
		thm = (Thumbnail *)elm_object_item_data_get(iter);

		if (thm->bChecked == true) {
			MSG_SEC("Found Checked Item: %s", ivug_get_filename((thm->path)));
			result = eina_list_append(result, iter);
		}

		iter = elm_gengrid_item_next_get(iter);
	}

	return result;

}

Eina_Bool
ivug_thumblist_checked_item_is(Evas_Object* obj)
{
	Elm_Object_Item *iter;
	Thumbnail *thm;

	iter = elm_gengrid_first_item_get(obj);

	while (iter != NULL) {
		thm = (Thumbnail *)elm_object_item_data_get(iter);

		if (thm->bChecked == true) {
			MSG_SEC("Found Checked Item: %s", ivug_get_filename((thm->path)));
			return EINA_TRUE;
		}

		iter = elm_gengrid_item_next_get(iter);
	}

	return EINA_FALSE;
}

