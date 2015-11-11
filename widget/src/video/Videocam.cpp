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

#include "Videocam.h"

static const char SIG_STARTED[] = "started";
static const char SIG_READY[] = "ready";
static const char SIG_EOS[] = "eos";


template<>
const Evas_Smart_Cb_Description CEvasSmartObject<CVideocam>::_signals[] = {
	{SIG_STARTED, ""},
	{SIG_READY, ""},
	{SIG_EOS, ""},
	{NULL, NULL}
};

void CVideocam::OnPlayerStarted(int w, int h)
{
	video_->SetSize(CSize(w, h));

	evas_object_smart_callback_call(GetObject(), SIG_STARTED, NULL);

	scroller_->SetClient(video_);
}


void CVideocam::OnPlayerReady(int w, int h)
{
//	video_->SetSize(CSize(w,h));

	evas_object_smart_callback_call(GetObject(), SIG_READY, NULL);

	scroller_->SetClient(video_);
}

void CVideocam::OnPlayerEOS(void)
{
	evas_object_smart_callback_call(GetObject(), SIG_EOS, NULL);

}

void CVideocam::move(int x, int y)
{
	CEvasSmartObject<CVideocam>::move(x, y);

	MSG_HIGH("Move to %d,%d", x, y);

	evas_object_move(scroller_->GetObject(), x, y);
}

void CVideocam::resize(int w, int h)
{
	CEvasSmartObject<CVideocam>::resize(w, h);

	MSG_HIGH("Resize to %d,%d", w, h);
	evas_object_resize(scroller_->GetObject(), w, h);
}

void CVideocam::show()
{
	CEvasSmartObject<CVideocam>::show();

	MSG_HIGH("Videcam Show");

	if (video_) {
		evas_object_show(video_->GetObject());
	}

	if (scroller_) {
		evas_object_show(scroller_->GetObject());
	}
}

void CVideocam::hide()
{
	CEvasSmartObject<CVideocam>::hide();

	MSG_HIGH("Videcam Hide");

	if (video_) {
		evas_object_hide(video_->GetObject());
	}

	if (scroller_) {
		evas_object_hide(scroller_->GetObject());
	}
}

void CVideocam::draw()
{
	CEvasSmartObject<CVideocam>::draw();

	if (scroller_) {
		scroller_->Draw();
	}
}

void CVideocam::remove()
{
	MSG_ERROR("Videocam Deleted");

	delete scroller_;
	scroller_ = NULL;

	EFL::dump_obj(GetObject() , 0);

	delete video_;
}

