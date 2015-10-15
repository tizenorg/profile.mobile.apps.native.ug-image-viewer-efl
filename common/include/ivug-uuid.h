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

#ifndef __IVUG_UUID_H__
#define __IVUG_UUID_H__

#include "ivug-datatypes.h"
#include <stdbool.h>



#ifdef __cplusplus
extern "C" {
#endif


UUID uuid_assign(UUID id);

void uuid_free(UUID id);

/*
	return 0 if identical
*/
int uuid_compare(UUID id1, UUID id2);

const char *uuid_getchar(UUID id);

UUID uuid_getuuid(const char *szID);

bool uuid_is_valid(UUID id);


#ifdef __cplusplus
}
#endif

#endif // __IVUG_UUID_H__

