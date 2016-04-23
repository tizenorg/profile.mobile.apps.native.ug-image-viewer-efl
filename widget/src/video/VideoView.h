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

#pragma once

#include "ScrollerClient.h"
//#include "ivug-player.h"


/*
	Signal.
		"video.started" with Evas_Point. x = stream width, y = stream height
		"video.eos"
		"video.paused",
		"video.resumed",
*/
class CVideoView : public CScrollerClient {
	static void _OnPlayerCB(PlayerParam *param, void *data)
	{
		CVideoView *pVideoView = static_cast<CVideoView *>(data);

		pVideoView->OnPlayerCB(param);
	}

	virtual void Draw() {
		const CRect &rect = GetClientRect();
		const CPoint &origin = GetOrigin();

		evas_object_move(m_video, origin.X() + rect.Left() , origin.Y() + rect.Top());
		evas_object_resize(m_video, rect.Width() , rect.Height());

	};

public:
	CVideoView(Evas_Object *parent) : CScrollerClient(parent), m_video(NULL), m_player(NULL) {
#if 1
		m_video = evas_object_image_add(evas_object_evas_get(GetObject()));
		evas_object_image_filled_set(m_video, EINA_FALSE);

		evas_object_name_set(m_video, "video");

		evas_object_smart_member_add(m_video, GetObject());
#else
		m_video = evas_object_image_filled_add(evas_object_evas_get(parent));
#endif
//		evas_object_size_hint_weight_set(m_video, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//		evas_object_size_hint_fill_set(m_video, EVAS_HINT_FILL, EVAS_HINT_FILL);
	};

	virtual ~CVideoView() {
		UnsetFile();
	};

	Evas_Load_Error SetFile(const char *file)
	{
		m_player = ivug_player_create();

		ivug_player_set_file(m_player, file);

		evas_object_image_size_set(m_video, 1, 1);		// Size set should be called before play

		ivug_player_set_callback(m_player, _OnPlayerCB, this);

		return EVAS_LOAD_ERROR_NONE;
	};

	void UnsetFile() {
		if ( m_player )
		{
			Stop();
			ivug_player_delete(m_player);

			m_player = NULL;
		}
	};

	void Play()
	{
		PlayerConfig config;

		config.bLooping = false;
		config.pSurface = m_video;
		config.eSurface = IVUG_PLAYER_SURFACE_EVAS;

		ivug_player_set_config(m_player, &config);

		ivug_player_start(m_player);
	}

	void Stop()
	{
		ivug_player_stop(m_player);
		evas_object_hide(m_video);
		evas_object_image_fill_set(m_video, 0,0,0,0);	// Reset
	}

	void Pause()
	{
		ivug_player_pause(m_player);
	}

	void Resume()
	{
		ivug_player_resume(m_player);
	}

	virtual void Zoom(double zoom) {
		CScrollerClient::Zoom(zoom);

		if ( evas_object_visible_get(m_video ) == EINA_TRUE )
		{
			const CRect &rect = GetClientRect();

			MSG_HIGH("Video Fill Set : (%d,%d)", rect.Width(), rect.Height());

			evas_object_image_fill_set(m_video, 0, 0, rect.Width(), rect.Height());
		}
	};

	bool IsPlaying()
	{
		PlayerState state;

		ivug_player_state_get(m_player, &state);

		if ( state == IVUG_PLAYER_STATE_PLAYING )
		{
			return true;
		}

		return false;

	}
private:
	void OnPlayerCB(PlayerParam *param)
	{
		Evas_Point pt;

		switch(param->type)
		{
			case IVUG_PLAYER_EVENT_STARTED:
				MSG_HIGH("Player Started. WH(%d,%d)", param->param.started.w, param->param.started.h);

				pt.x = param->param.started.w;
				pt.y = param->param.started.h;

				evas_object_show(m_video);

				evas_object_smart_callback_call(GetObject(), "video.started", (void *)&pt);

				break;

			case IVUG_PLAYER_EVENT_PREPARED:
				MSG_HIGH("Player Prepared.");

				evas_object_smart_callback_call(GetObject(), "video.ready", (void *)NULL);

				break;

			case IVUG_PLAYER_EVENT_EOS:
				evas_object_hide(m_video);
				evas_object_image_fill_set(m_video, 0,0,0,0);	// Reset
				evas_object_smart_callback_call(GetObject(), "video.eos", (void *)&pt);
				break;
			case IVUG_PLAYER_EVENT_ERROR:
				break;
			default:
				break;
			}
	}
private:
	Evas_Object *m_video;

	PlayerHandle m_player;

};


