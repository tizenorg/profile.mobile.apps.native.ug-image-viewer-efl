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

#include "ivug-video.h"

#include "debug.h"
#include "Videocam.h"


#define VIDEO_CLASS(obj) \
	static_cast<CVideocam *>(evas_object_data_get((obj), "CVideocam"))


IVAPI Evas_Object *ivug_video_create(Evas_Object *parent)
{
	CVideocam *video = new CVideocam;

	Evas_Object *obj = video->CreateObject(parent);

	evas_object_data_set(obj, "CVideocam", video);

	MSG_HIGH("Create videocam object. class=0x%08x obj=0x%08x", video, obj);
	return obj;
}

IVAPI Evas_Load_Error ivug_video_file_set(Evas_Object *obj, const char *file)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	video->SetFile(file);

	return EVAS_LOAD_ERROR_NONE;
}

IVAPI Evas_Load_Error ivug_video_file_unset(Evas_Object *obj)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	video->UnSet();

	return EVAS_LOAD_ERROR_NONE;
}

IVAPI Eina_Bool ivug_video_play(Evas_Object *obj)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	video->Play();

	return EINA_TRUE;
}

IVAPI Eina_Bool ivug_video_stop(Evas_Object *obj)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	video->Stop();

	return EINA_TRUE;
}

IVAPI Eina_Bool ivug_video_pause(Evas_Object *obj)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	video->Pause();

	return EINA_TRUE;
}

IVAPI Eina_Bool ivug_video_resume(Evas_Object *obj)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	video->Resume();

	return EINA_TRUE;
}


IVAPI void ivug_video_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);


	const CRect &rect = video->GetDisplayGeometry();

	if(x) *x = rect.Left();
	if(y) *y = rect.Top();
	if(w) *w = rect.Width();
	if(h) *h = rect.Height();

	MSG_HIGH("Region size XYWH(%d,%d,%d,%d)", rect.Left(), rect.Top(), rect.Width(), rect.Height() );
}

IVAPI void ivug_video_hold_set(const Evas_Object *obj, Eina_Bool hold)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	if (hold == EINA_TRUE) {
		video->DisableEvent();
	} else {
		video->EnableEvent();
	}

}

IVAPI void ivug_video_zoom_reset(Evas_Object *obj, Evas_Point *pCenter)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	video->DoAutoFit();
}

IVAPI Eina_Bool ivug_video_is_playing(Evas_Object *obj)
{
	CVideocam *video = VIDEO_CLASS(obj);
	MSG_ASSERT(video != NULL);

	if (video->IsPlaying() == true ) {
		return  EINA_TRUE;
	}

	return	EINA_FALSE;
}
