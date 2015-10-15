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

#include "EvasSmartObj.h"
#include "VideoView.h"
#include "EFLUtil.h"


#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-VIDEO"


class CVideocam : public CEvasSmartObject<CVideocam> {
	static void _OnPlayerStarted(void *data, Evas_Object *obj, void *event_info)
	{
		CVideocam *pVideoCam = static_cast<CVideocam *>(data);

		Evas_Point *pPoint = static_cast<Evas_Point *>(event_info);

		pVideoCam->OnPlayerStarted(pPoint->x, pPoint->y);
	}

	static void _OnPlayerReady(void *data, Evas_Object *obj, void *event_info)
	{
		CVideocam *pVideoCam = static_cast<CVideocam *>(data);

		pVideoCam->OnPlayerReady(0,0);
	}

	static void _OnPlayerEOS(void *data, Evas_Object *obj, void *event_info)		// End of stream
	{
		CVideocam *pVideoCam = static_cast<CVideocam *>(data);

		pVideoCam->OnPlayerEOS();
	}

public:
	CVideocam() : CEvasSmartObject<CVideocam>(), scroller_(NULL), video_(NULL), bMotion(false) {
			CEvasSmartObject<CVideocam>::SetClassName("Videocam");
	};

	~CVideocam() {};

	Evas_Object *CreateObject(Evas_Object *parent) {
		CEvasSmartObject<CVideocam>::CreateObject(parent);

		evas_object_smart_callbacks_descriptions_set(GetObject(), _signals);

		scroller_ = new CScroller("vScroller");
		scroller_->CreateObject(GetObject());

		evas_object_smart_member_add(scroller_->GetObject(), GetObject());

		evas_object_name_set(GetObject(), "videocam");

		return GetObject();
	};

	Evas_Load_Error SetFile(const char *file)
	{
		UnSet();

		Evas_Load_Error error;

		video_ = new CVideoView(GetObject());
		error = video_->SetFile(file);

		MSG_HIGH("Set file in player : %s", file);

		evas_object_smart_callback_add(video_->GetObject(), "video.started", _OnPlayerStarted, this);
		evas_object_smart_callback_add(video_->GetObject(), "video.ready", _OnPlayerReady, this);
		evas_object_smart_callback_add(video_->GetObject(), "video.eos", _OnPlayerEOS, this);

		return error;
	};

	void UnSet() {
		// Scroller Client should be removed
		scroller_->UnsetClient();

		if ( video_ ) delete video_;
		video_ = NULL;
	};

	void Play()
	{
		MSG_HIGH("Play");
		video_->Play();
	}

	void Stop()
	{
		MSG_HIGH("Stop");
		video_->Stop();
	}

	void Pause()
	{
		MSG_HIGH("Pause");
		video_->Pause();
	}

	void Resume()
	{
		MSG_HIGH("Resume");
		video_->Resume();
	}

	void OnPlayerStarted(int w, int h);
	void OnPlayerReady(int w, int h);
	void OnPlayerEOS(void);


	CRect GetDisplayGeometry() const {
		// Original image size;

		if ( video_ == NULL )
		{
			return CRect::Zero;
		}

		const CRect &rect = video_->GetClientRect();
		const CPoint &origin = video_->GetOrigin();

		MSG_HIGH("Display Rect : Origin(%d,%d) Client(%d,%d,%d,%d)", origin.X() , origin.Y() , rect.Left(), rect.Top(), rect.Width(), rect.Height());
		return CRect(/*origin.X() + */ rect.Left(), /* origin.Y() + */ rect.Top(), rect.Width(), rect.Height());
	};

	void DisableEvent() const { 	scroller_->DisableEvent(); };
	void EnableEvent() const  { 	scroller_->EnableEvent(); };

	void DoAutoFit() {
		scroller_->DoAutoFit();
	};

	void SetTiltMotion(bool bUse) {
		if ( bUse == true )
		{
			scroller_->EnableTiltMotion();
		}
		else
		{
			scroller_->DisableTiltMotion();
		}
	}

	void SetPanMotion(bool bUse) {
		if ( bUse == true )
		{
			scroller_->EnablePanMotion();
		}
		else
		{
			scroller_->DisablePanMotion();
		}

	}

	bool IsPlaying() {
		if ( video_ == NULL )
		{
			return false;
		}

		return video_->IsPlaying();
	};

private:
	void move(int x, int y);
	void resize(int w, int h);
	void show();
	void hide();
	void draw();
	void remove();

private:
	CScroller *scroller_;
	CVideoView *video_;
	bool bMotion;
};


