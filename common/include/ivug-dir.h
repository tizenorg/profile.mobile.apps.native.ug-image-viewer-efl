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

#ifndef __IVUG_DIR_H__
#define __IVUG_DIR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*EnumFilesCB)(const char *fname, void *data);


/*
	Enumerate file name in @dir & call user callback function indicated by @pFunc..

	Synchrounous function.
*/
bool EnumFilesInDir(const char *dir, EnumFilesCB pFunc, void *data);

/*
	Retrieve file count in @dir.
	return -1 when error occured.
*/
int GetFilesCountInDir(const char *dir);


#ifdef __cplusplus
}
#endif


#endif 		// __IVUG_DIR_H__

