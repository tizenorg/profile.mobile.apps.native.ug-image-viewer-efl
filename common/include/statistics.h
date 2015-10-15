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

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int iv_ta_accum_item_begin(int lvl, const char* name, bool show, const char* filename, int line);
int iv_ta_accum_item_end(int lvl, const char* name, bool show, const char* filename, int line);
void iv_ta_accum_show_result_fp(FILE *fp);

int IV_PERF_INIT();
int IV_PERF_DEINIT();


#ifdef __cplusplus
}
#endif

#endif /* __STATISTICS_H__ */

