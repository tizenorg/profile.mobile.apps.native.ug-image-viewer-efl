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

#include "ivug-comment.h"

#include "jpeg.h"

using namespace iv;

extern "C" Handle ivug_comment_loadfile(const char *fname)
{
	MSG_ASSERT(fname != NULL);

	CJPEG *pJpeg = new CJPEG();

	if (pJpeg->ParseJPEG(fname) == false) {
		delete pJpeg;
		return NULL;
	}

	return (Handle)pJpeg;
}


extern "C" const char *ivug_comment_get(Handle handle)
{
	MSG_ASSERT(handle != NULL);

	return ((CJPEG *)handle)->GetUserComment();
}


extern "C" void ivug_comment_set(Handle handle, const char *comment)
{
	MSG_ASSERT(handle != NULL);

	CJPEG *pJpeg = (CJPEG *)handle;

	pJpeg->SetUserComment(comment);
}


extern "C" void ivug_comment_savefile(Handle handle, const char *fname)
{
	MSG_ASSERT(handle != NULL);

	CJPEG *pJpeg = (CJPEG *)handle;

	pJpeg->WriteJPEG(fname);
}


extern "C" void ivug_comment_closefile(Handle handle)
{
	MSG_ASSERT(handle != NULL);

	CJPEG *pJpeg = (CJPEG *)handle;

	delete pJpeg;
}
