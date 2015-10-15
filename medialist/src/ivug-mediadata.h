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

#ifndef __IVUG_MEDIADATA_H__
#define __IVUG_MEDIADATA_H__

#include "ivug-define.h"

#include "ivug-media.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * Converting Media_Data from filepath
  * @param filepath[in]
  *
**/
Media_Data *ivug_alloc_mediadata_from_filepath(const char* filepath);

/**
  * Converting WMitem to Media_Data
  * @param item[in]
  *
**/

Media_Data *ivug_alloc_mediadata_from_media_handle(media_handle media);

Media_Data *ivug_alloc_mediadata_from_url(const char *url);

/**
  * Free memory used by Media_Data
  * @param mdata[in]
  *
**/
void ivug_free_mediadata(Media_Data* mdata);

#ifdef __cplusplus
}
#endif

#endif // __IVUG_MEDIADATA_H__
