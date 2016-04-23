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

#include "ivug-dir.h"

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

#include "ivug-debug.h"
#include <linux/limits.h>

bool EnumFilesInDir(const char *dir, EnumFilesCB pFunc, void *data)
{
	struct dirent ent_struct;
	struct dirent *dptr;

	DIR *dirp;

	if ((dirp = opendir(dir)) == NULL) {
		MSG_UTIL_ERROR("Cannot open dir : %s", dir);
		return false;
	}

	char fullpath[PATH_MAX];

	while ((readdir_r(dirp, &ent_struct, &dptr) == 0) && dptr) {
		if (dptr->d_type == DT_REG) {		// Only for gegular file
			if (pFunc) {
				snprintf(fullpath, PATH_MAX, "%s/%s", dir, dptr->d_name);
				pFunc(fullpath, data);
			}
		}
	}

	closedir(dirp);

	return true;
}


int GetFilesCountInDir(const char *dir)
{
	struct dirent ent_struct;
	struct dirent *dptr;
	DIR *dirp;

	if ((dirp = opendir(dir)) == NULL) {
		MSG_UTIL_ERROR("Cannot open dir : %s", dir);
		return -1;
	}

	int nCount = 0;

	while ((readdir_r(dirp, &ent_struct, &dptr) == 0) && dptr) {
		if (dptr->d_type == DT_REG) {		// Only for gegular file
			nCount++;
		}
	}

	closedir(dirp);

	return nCount;
}
