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

#include <app.h>
#include "ivug-define.h"

#include "ivug-decolayer.h"
#include "ivug-debug.h"
#include "ivug-config.h"

#include <Ecore.h>
#include <algorithm>    // std::swap

#undef LOG_LVL
#define LOG_LVL (DBG_MSG_LVL_HIGH)

#undef LOG_CAT
#define LOG_CAT "IV-DECO"

#include "EvasSmartObj.h"
#include "iv-button.h"

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
#define IMG_PATH						full_path(UG_RES_PATH, "edje/images")

#define ICON_PLAY_SIZE 100

#define IMG_MARGIN (20)
#define TOP_OFFSET (80)

#define THUMB_ICON_SIZE (60)
#define ICON_EDJ_PATH full_path(EDJ_PATH, "/ivug-icons.edj")


static const char *szPlaySpeedIcon[] = {
	"icon.burstspeed.1.5th", 	"icon.burstspeed.1.5th.press",
	"icon.burstspeed.1.2th", 	"icon.burstspeed.1.2th.press",
	"icon.burstspeed.1", 		"icon.burstspeed.1.press",
	"icon.burstspeed.2", 		"icon.burstspeed.2.press",
};

class CDecolayer;
template<>
const Evas_Smart_Cb_Description CEvasSmartObject<CDecolayer>::_signals[] = {
	{"start,blink", ""},
	{"stop,blink", ""},
	{NULL, NULL}
};


class CDecolayer : public CEvasSmartObject<CDecolayer>
{
	static Eina_Bool _timer_cb(void *data) {
		CDecolayer *thiz = (CDecolayer *)data;

		thiz->OnTimer();

		return ECORE_CALLBACK_RENEW;
	}

#if (LOG_LVL & DBG_MSG_LVL_DEBUG)
	static void _on_receive_mouse_event(void *data, Evas *e, Evas_Object *obj, void *event_info) {
		char *str = (char *)data;

		MSG_HIGH("Decolayer(0x%08x):%s %s", obj, str, evas_object_name_get(obj));
	}
#endif

public:
	CDecolayer() : CEvasSmartObject<CDecolayer>(), icon_w(THUMB_ICON_SIZE), icon_h(THUMB_ICON_SIZE), bVisible(false), m_event(NULL), m_ico_soundpic(NULL), m_ico_play(NULL), m_ico_burstplay(NULL), m_ico_burstspeed(NULL) {
		CEvasSmartObject<CDecolayer>::SetClassName("DecoLayer");

		m_timer = NULL;
		m_deco = IVUG_DECO_NONE;
		MSG_HIGH("CDecolayer constructor");
	};

	virtual ~CDecolayer() {
		MSG_HIGH("CDecolayer destructor");
	};

	Evas_Object *CreateObject(Evas_Object *parent) {
		CEvasSmartObject<CDecolayer>::CreateObject(parent);
// Register signal
		evas_object_smart_callbacks_descriptions_set(GetObject(), _signals);

		m_event = evas_object_rectangle_add(evas_object_evas_get(parent));
		evas_object_pass_events_set(m_event, EINA_TRUE);
		evas_object_color_set(m_event, 0, 0, 0, 0);

		evas_object_smart_member_add(m_event, GetObject());

#if (LOG_LVL & DBG_MSG_LVL_DEBUG)
		evas_object_event_callback_add(m_event, EVAS_CALLBACK_MOUSE_DOWN, _on_receive_mouse_event, "decolayer.down");
		evas_object_event_callback_add(m_event, EVAS_CALLBACK_MOUSE_UP, _on_receive_mouse_event, "decolayer.up");
#endif

		evas_object_show(m_event);
		return GetObject();
	};

	void UpdateIcon() {
		MSG_MED("Update Icon!");
		SetType(m_deco, true);
	}

// IMG_PATH"/bestpic/gallery_icon_bestpic.png"
	bool SetType(Ivug_Deco deco, bool bForce = false) {
		MSG_HIGH("Set decoration type(%d --> %d) bForce=%d", m_deco, deco, bForce);

		if (bForce == false) {
			if (m_deco == deco) {
				return true;
			}
		}

		switch (m_deco) {	// Old type
		case IVUG_DECO_VIDEO:
			if (m_ico_play) {
				evas_object_hide(m_ico_play);
			}
			break;
		case IVUG_DECO_PANORAMA:
			break;
		case IVUG_DECO_SOUNDPIC:
			if (m_timer) {	// Remove soundpic timer..
				ecore_timer_del(m_timer);
				m_timer = NULL;
			}

			if (m_ico_soundpic) {
				evas_object_hide(m_ico_soundpic);
			}
			break;
		case IVUG_DECO_BURST:
			if (m_ico_burstplay) {
				evas_object_hide(m_ico_burstplay);
			}

			if (m_ico_burstspeed) {
				evas_object_hide(m_ico_burstspeed);
			}

			break;

		case IVUG_DECO_NONE:
			if (m_ico_play) {
				evas_object_hide(m_ico_play);
			}
			if (m_ico_soundpic) {
				evas_object_hide(m_ico_soundpic);
			}

			if (m_ico_burstplay) {
				evas_object_hide(m_ico_burstplay);
			}

			if (m_ico_burstspeed) {
				evas_object_hide(m_ico_burstspeed);

			}

			break;
		default:
			MSG_ERROR("Unknown media type : %d", m_deco);
			return false;
		}

		m_deco = deco;

		if (m_deco == IVUG_DECO_BURST) {
			eConfigPlaySpeed cState = PLAYSPEED_UNDEFINED;

			/*if (ivug_config_get_playspeed(&cState) == false) {
				cState = PLAYSPEED_1;
			}*/

			if (m_speed != cState) {
				evas_object_del(m_ico_burstspeed);		// Can change plaspeed icon. so remove object
				m_ico_burstspeed = NULL;

				m_speed = cState;
			}
		}

		evas_object_smart_changed(GetObject());

		return true;

	}


	bool StartBlink() {
		MSG_MED("Start Blinking");
		if (m_timer) {
			ecore_timer_del(m_timer);
			m_timer = NULL;
		}

		m_timer = ecore_timer_add(0.6f, _timer_cb, this);

		return true;
	};


	bool StopBlink() {
		MSG_MED("Stop Blinking");
		if (m_timer) {
			ecore_timer_del(m_timer);
			m_timer = NULL;
		}

		if (m_ico_soundpic && (bVisible == true)) {
			evas_object_show(m_ico_soundpic);
		}

		return true;
	}

	bool CheckVIcon(int cx, int cy) {
		if (m_ico_play == NULL) {
			return false;
		}
		if (m_deco != IVUG_DECO_VIDEO) {
			return false;
		}

		int x, y, w, h;

		evas_object_geometry_get(m_ico_play, &x, &y, &w, &h);

		MSG_HIGH("Check video icon Geo(%d,%d,%d,%d) Point(%d,%d)", x, y, w, h, cx, cy);
		if ((x < cx) && (cx < (x + w)) && (y < cy) && (cy < (y + h))) {
			return true;
		}

		return false;
	}

	bool CheckSoundIcon(int cx, int cy) {
		if (m_ico_soundpic == NULL) {
			return false;
		}
		if (m_deco != IVUG_DECO_SOUNDPIC) {
			return false;
		}

		int x, y, w, h;

		evas_object_geometry_get(m_ico_soundpic, &x, &y, &w, &h);

		MSG_HIGH("Check Sound icon Geo(%d,%d,%d,%d) Point(%d,%d)", x, y, w, h, cx, cy);
		if ((x < cx) && (cx < (x + w)) && (y < cy) && (cy < (y + h))) {
			return true;
		}

		return false;
	}

	ivug_deco_icon_e CheckIcon(int cx, int cy) {
		int x, y, w, h;

		switch (m_deco) {
		case IVUG_DECO_SOUNDPIC:
			if (m_ico_soundpic == NULL) {
				MSG_WARN("Debug ME!! m_deco:%d", m_deco);
				return IVUG_DECO_ICON_NONE;
			}

			evas_object_geometry_get(m_ico_soundpic, &x, &y, &w, &h);

			MSG_HIGH("Check Sound icon Geo(%d,%d,%d,%d) Point(%d,%d)", x, y, w, h, cx, cy);
			if ((x < cx) && (cx < (x + w)) && (y < cy) && (cy < (y + h))) {
				return IVUG_DECO_ICON_SOUNDPIC;
			}

			break;
		case IVUG_DECO_BURST:
			if (m_ico_burstplay == NULL) {
				MSG_WARN("Debug ME!! m_deco:%d", m_deco);
				return IVUG_DECO_ICON_NONE;
			}

			if (m_ico_burstspeed == NULL) {
				MSG_WARN("Debug ME!! m_deco:%d", m_deco);
				return IVUG_DECO_ICON_NONE;
			}

			evas_object_geometry_get(m_ico_burstplay, &x, &y, &w, &h);

			MSG_HIGH("Check Burst icon play Geo(%d,%d,%d,%d) Point(%d,%d)", x, y, w, h, cx, cy);
			if ((x < cx) && (cx < (x + w)) && (y < cy) && (cy < (y + h))) {
				return IVUG_DECO_ICON_BURST_PLAY;
			}

			evas_object_geometry_get(m_ico_burstspeed, &x, &y, &w, &h);

			MSG_HIGH("Check Burst speed icon Geo(%d,%d,%d,%d) Point(%d,%d)", x, y, w, h, cx, cy);
			if ((x < cx) && (cx < (x + w)) && (y < cy) && (cy < (y + h))) {
				return IVUG_DECO_ICON_BURST_PLAYSPEED;
			}


			break;
		case IVUG_DECO_VIDEO:
			if (m_ico_play == NULL) {
				MSG_WARN("Debug ME!! m_deco:%d", m_deco);
				return IVUG_DECO_ICON_NONE;
			}

			evas_object_geometry_get(m_ico_play, &x, &y, &w, &h);

			MSG_HIGH("Check video icon Geo(%d,%d,%d,%d) Point(%d,%d)", x, y, w, h, cx, cy);
			if ((x < cx) && (cx < (x + w)) && (y < cy) && (cy < (y + h))) {
				return IVUG_DECO_ICON_VIDEO;
			}

			break;
		default:

			break;
		}


		return IVUG_DECO_ICON_NONE;

	}

	void hide_icon_play() {
		if (m_ico_play) {
			evas_object_hide(m_ico_play);
		}
	}

protected:
	virtual void clip_set(Evas_Object *clipper) {

		if (m_ico_soundpic) {
			evas_object_clip_set(m_ico_soundpic, clipper);
		}

		if (m_ico_play) {
			evas_object_clip_set(m_ico_play, clipper);
		}

		if (m_ico_burstplay) {
			evas_object_clip_set(m_ico_burstplay, clipper);
		}

		if (m_ico_burstspeed) {
			evas_object_clip_set(m_ico_burstspeed, clipper);
		}

	};

	virtual void clip_unset() {

		if (m_ico_soundpic) {
			evas_object_clip_unset(m_ico_soundpic);
		}

		if (m_ico_play) {
			evas_object_clip_unset(m_ico_play);
		}

		if (m_ico_burstplay) {
			evas_object_clip_unset(m_ico_burstplay);
		}

		if (m_ico_burstspeed) {
			evas_object_clip_unset(m_ico_burstspeed);
		}

	};

	virtual void draw() {
		CRect m_rect = GetGeometry();

		MSG_LOW("Draw XYWH(%d,%d,%d,%d) Visible=%d", m_rect.Left(), m_rect.Top(), m_rect.Width(), m_rect.Height(), bVisible);

		if (m_deco == IVUG_DECO_NONE) {
			MSG_LOW("No need to draw. Deco is NONE");
			return ;
		}

		if (m_rect.Width() == 0 || m_rect.Height() == 0) {
			MSG_WARN("No need to draw");
			return ;
		}

		if (bVisible == false) {
			MSG_WARN("Decolayer visible is false");
			return;
		}

// smart_member_add()�� _del() �� ȭ�� blinking �� ����Ű�� �� ��.
		if (m_deco == IVUG_DECO_SOUNDPIC) {
			if (m_ico_soundpic == NULL) {
//				m_ico_soundpic = load_icon(ICON_EDJ_PATH, "icon.sound_scene");

				iv::CButton *pBtn = iv::CButton::ObjectFactory(GetObject());

				pBtn->SetImage(
				    load_icon(ICON_EDJ_PATH, "icon.sound_scene"),
				    load_icon(ICON_EDJ_PATH, "icon.sound_scene.press"),
				    load_icon(ICON_EDJ_PATH, NULL)
				);

				m_ico_soundpic = pBtn->GetObject();

				evas_object_name_set(m_ico_soundpic, "IconSound");
				evas_object_smart_member_add(m_ico_soundpic, GetObject());
			}

			int x, y;

			x = std::max(m_rect.Left(), 0) + IMG_MARGIN;
			y = std::max(m_rect.Top(), TOP_OFFSET) + IMG_MARGIN;

			MSG_MED("Draw Sound XY(%d,%d)", x, y);
			evas_object_resize(m_ico_soundpic, icon_w, icon_h);
			evas_object_move(m_ico_soundpic, x, y);

			if (bVisible == true) {
				evas_object_show(m_ico_soundpic);
			}
		}

		if (m_deco == IVUG_DECO_BURST) {
			if (m_ico_burstplay == NULL) {
//				m_ico_burstplay = load_icon(ICON_EDJ_PATH, "icon.burst");

				iv::CButton *pBtn = iv::CButton::ObjectFactory(GetObject());

				pBtn->SetImage(
				    load_icon(ICON_EDJ_PATH, "icon.burst"),
				    load_icon(ICON_EDJ_PATH, "icon.burst.press"),
				    load_icon(ICON_EDJ_PATH, "icon.burst.dim")
				);

				m_ico_burstplay = pBtn->GetObject();

				evas_object_name_set(m_ico_burstplay, "IconBurstPlay");
				evas_object_smart_member_add(m_ico_burstplay, GetObject());
			}

			if (m_ico_burstspeed == NULL) {
//				m_ico_burstspeed = load_icon(ICON_EDJ_PATH, "icon.burstspeed.1.5th");

				iv::CButton *pBtn = iv::CButton::ObjectFactory(GetObject());

				pBtn->SetImage(
				    load_icon(ICON_EDJ_PATH, szPlaySpeedIcon[((int)m_speed - 1) * 2]),
				    load_icon(ICON_EDJ_PATH, szPlaySpeedIcon[((int)m_speed - 1) * 2 + 1]),
				    NULL
				);

				m_ico_burstspeed = pBtn->GetObject();

				evas_object_name_set(m_ico_burstspeed, "IconBurstSpeed");
				evas_object_smart_member_add(m_ico_burstspeed, GetObject());
			}

			int x, y;

			x = std::min((std::max(m_rect.Left(), 0) + IMG_MARGIN), m_rect.Right() - (IMG_MARGIN + icon_w + IMG_MARGIN + icon_w + IMG_MARGIN));
			y = std::max(m_rect.Top(), TOP_OFFSET) + IMG_MARGIN;

			MSG_MED("Draw Burst Play XY(%d,%d)", x, y);
			evas_object_resize(m_ico_burstplay, icon_w, icon_h);
			evas_object_move(m_ico_burstplay, x, y);

			MSG_MED("Draw Burst Speed XY(%d,%d)", m_rect.Left() + IMG_MARGIN, m_rect.Top() + IMG_MARGIN + TOP_OFFSET);
			evas_object_resize(m_ico_burstspeed, icon_w, icon_h);
			evas_object_move(m_ico_burstspeed, x + icon_w + IMG_MARGIN, y);

			if (bVisible == true) {
				evas_object_show(m_ico_burstplay);
				evas_object_show(m_ico_burstspeed);
			}
		}
	};

	virtual void remove() {
		MSG_HIGH("Remove Decolayer(0x%08x)", this);

		if (m_timer) {
			ecore_timer_del(m_timer);
			m_timer = NULL;
		}

		if (m_ico_soundpic) {
			evas_object_smart_member_del(m_ico_soundpic);
			evas_object_del(m_ico_soundpic);
			m_ico_soundpic = NULL;
		}

		if (m_ico_play) {
			evas_object_smart_member_del(m_ico_play);
			evas_object_del(m_ico_play);
			m_ico_play = NULL;
		}

		if (m_ico_burstplay) {
			evas_object_smart_member_del(m_ico_burstplay);
			evas_object_del(m_ico_burstplay);
			m_ico_burstplay = NULL;
		}

		if (m_ico_burstspeed) {
			evas_object_smart_member_del(m_ico_burstspeed);
			evas_object_del(m_ico_burstspeed);
			m_ico_burstspeed = NULL;
		}

		if (m_event) {
			evas_object_del(m_event);
			m_event = NULL;
		}

		delete this;
	};

	virtual void show() {
		MSG_MED("Show!");

		bVisible = true;
	}

	virtual void hide() {
		MSG_MED("Hide!");

		bVisible = false;

		if (m_ico_soundpic) {
			evas_object_hide(m_ico_soundpic);
		}

		if (m_ico_burstplay) {
			evas_object_hide(m_ico_burstplay);
		}

		if (m_ico_burstspeed) {
			evas_object_hide(m_ico_burstspeed);
		}


	}

	virtual void move(int x, int y) {
		CEvasSmartObject<CDecolayer>::move(x, y);

		MSG_LOW("Moved XY(%d,%d)", x, y);

		if (m_event) {
			evas_object_move(m_event, x, y);
		}
		evas_object_smart_changed(GetObject());

	};

	virtual void resize(int w, int h) {
		CEvasSmartObject<CDecolayer>::resize(w, h);

		MSG_LOW("Resized WH(%d,%d)", w, h);

		if (m_event) {
			evas_object_resize(m_event, w, h);
		}

		evas_object_smart_changed(GetObject());

	};

private:

	void OnTimer() {
		if (m_ico_soundpic == NULL) {
			return;
		}

		if (bVisible == true) {
			if (evas_object_visible_get(m_ico_soundpic) == false) {
				evas_object_show(m_ico_soundpic);
			} else {
				evas_object_hide(m_ico_soundpic);
			}
		}
	}

	Evas_Object *create_icon(Evas_Object *parent, const char *edjname, const char *groupname)
	{
		Evas_Object *icon;

		icon = elm_icon_add(parent);

		if (elm_image_file_set(icon, edjname, groupname) == EINA_FALSE) {
			MSG_ERROR("Cannot file set. EDJ=%s Group=%s", edjname, groupname);
			evas_object_del(icon);
			return NULL;
		}

		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_image_resizable_set(icon, 1, 1);
		evas_object_size_hint_expand_set(icon, 1, 1);

		return icon;
	}

	Evas_Object *load_icon(const char *edj_file, const char *part) {
		Evas_Object *icon;

		icon = create_icon(GetObject(), edj_file, part);

		if (icon == NULL) {
			MSG_ERROR("Cannot load icon : %s,%s", edj_file, part);
			return NULL;
		}

		int iw, ih;
		elm_image_object_size_get(icon, &iw, &ih);

		MSG_MED("Icon(%s,%s) Size WH(%d,%d)", edj_file, part, iw, ih);

		return icon;
	}
private:
	int icon_w;
	int icon_h;

	bool bVisible;

	Evas_Object *m_event;

	Evas_Object *m_ico_soundpic;
	Evas_Object *m_ico_play;

	Evas_Object *m_ico_burstplay;
	Evas_Object *m_ico_burstspeed;

	Ecore_Timer *m_timer;
	Ivug_Deco m_deco;

	eConfigPlaySpeed m_speed;
};


#define DECO_CLASS(obj) \
	static_cast<CDecolayer *>(evas_object_data_get((obj), "CDecolayer"))


Evas_Object *ivug_decolayer_add(Evas_Object *parent)
{
	CDecolayer *klass = new CDecolayer;

	Evas_Object *obj = klass->CreateObject(parent);
	MSG_ASSERT(obj != NULL);

	evas_object_data_set(obj, "CDecolayer", klass);

	MSG_HIGH("Create decolayer object. class=0x%08x obj=0x%08x", klass, obj);
	return obj;
}

void ivug_decolayer_set_type(Evas_Object *obj, Ivug_Deco deco)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);

	klass->SetType(deco);
}

void ivug_decolayer_start_blinking(Evas_Object *obj)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);

	klass->StartBlink();
}

void ivug_decolayer_stop_blinking(Evas_Object *obj)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);

	klass->StopBlink();
}

bool ivug_decolayer_check_video_icon(Evas_Object *obj, int cx, int cy)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);

	return klass->CheckVIcon(cx, cy);
}


bool ivug_decolayer_check_sound_icon(Evas_Object *obj, int cx, int cy)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);

	return klass->CheckSoundIcon(cx, cy);
}

ivug_deco_icon_e ivug_decolayer_check_icon(Evas_Object *obj, int cx, int cy)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);

	return klass->CheckIcon(cx, cy);
}

void ivug_decolayer_update_icon(Evas_Object *obj)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);

// Force update
	klass->UpdateIcon();
}

void ivug_decolayer_hide_play_icon(Evas_Object *obj)
{
	CDecolayer *klass = DECO_CLASS(obj);
	MSG_ASSERT(klass != NULL);
	klass->hide_icon_play();
}
