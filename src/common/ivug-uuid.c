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

#include "ivug-uuid.h"
#include <stdio.h>		// for NULL
#include <stdlib.h>		// free
#include <string.h>		// strdup

inline UUID uuid_assign(UUID id)
{
#ifdef _USE_MEDIAINFO_STRINGID_
	if (id == NULL)
		return NULL;

	return strdup(id);
#else
	return id;
#endif
}


inline void uuid_free(UUID id)
{
#ifdef _USE_MEDIAINFO_STRINGID_
	if (id != NULL)
		free((void *)id);
#endif
}


inline int uuid_compare(UUID id1, UUID id2)
{
#ifdef _USE_MEDIAINFO_STRINGID_
	return strcmp(id1, id2);
#else
	return id1 - id2;
#endif
}

inline const char *uuid_getchar(UUID id)
{
#ifdef _USE_MEDIAINFO_STRINGID_
	if (id == NULL)
	{
		return "NULL";
	}

	return id;
#else
	{
		static char buffer[255];

		snprintf(buffer, 255, "%d", id);

		return buffer;
	}
#endif

}


inline UUID uuid_getuuid(const char *szID)
{
	if (szID == NULL)
	{
		return INVALID_UUID;
	}

#ifdef _USE_MEDIAINFO_STRINGID_
	return strdup(szID);
#else
	return ivug_atoi(szID);
#endif

}

inline bool uuid_is_valid(UUID id)
{
	return (id != INVALID_UUID) ? true : false;
}



