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

#include "ivug-define.h"
#include "ivug-debug.h"

#include "ivug-media.h"

#include <string.h>

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-MINFO"

static bool _is_bestpic(const char *file)
{
	const char szBest[] = "_bestshot";		// sizeof(szBest) = 10;

	const char *find = NULL;

	find = strrchr(file, '.');

	if (find == NULL) {
		// All files should have extension
		return false;
	}

	unsigned int dist = find - file;

	if (dist < sizeof(szBest) - 1) {
		return false;
	}

	if (strncmp(find - sizeof(szBest) + 1 , szBest , sizeof(szBest) - 1) == 0) {
		return true;
	}

	return false;

}


static bool _is_soundscene(const char *file)
{
	return false;
}

extern "C" MImageType MINfo_GetMediaType(const char *fname)
{
	if (_is_bestpic(fname) == true) {
		MSG_HIGH("Bestphoto : %s", fname);
		return MIMAGE_TYPE_BESTSHOT;
	}

	if (_is_soundscene(fname) == true) {
		MSG_HIGH("Soundscene : %s", fname);
		return MIMAGE_TYPE_SOUNDSCENE;
	}

	return MIMAGE_TYPE_NORMAL;
}



