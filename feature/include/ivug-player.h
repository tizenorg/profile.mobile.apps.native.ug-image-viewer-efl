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

#ifndef __IVUG_PLAYER_H__
#define __IVUG_PLAYER_H__

#include <Evas.h>

typedef void * PlayerHandle;

typedef enum {
	IVUG_PLAYER_SURFACE_NONE,
	IVUG_PLAYER_SURFACE_EVAS,
	IVUG_PLAYER_SURFACE_X11,
} PlayerSurface;

typedef struct {
	bool bLooping;

	PlayerSurface eSurface;
	Evas_Object *pSurface;
} PlayerConfig;

typedef enum {
	IVUG_PLAYER_EVENT_ERROR,
	IVUG_PLAYER_EVENT_PREPARED,
	IVUG_PLAYER_EVENT_STARTED,		// Playing
	IVUG_PLAYER_EVENT_PAUSED,
	IVUG_PLAYER_EVENT_EOS,
} PlayerEvent;

typedef enum {
	IVUG_PLAYER_STATE_NONE,
	IVUG_PLAYER_STATE_CREATED,
	IVUG_PLAYER_STATE_PLAYING,
	IVUG_PLAYER_STATE_PAUSED,
} PlayerState;

typedef struct {
	PlayerEvent type;

	union {

		struct {	// IVUG_PLAYER_EVENT_STARTED
			int w;
			int h;
		} started;

		struct {
			int current;
		} pos;

		struct {	// IVUG_PLAYER_EVENT_ERROR
			int error_code;
		} error;
	} param;
} PlayerParam;

typedef void (*PlayerCallback)(PlayerParam *param, void *data);

#ifdef __cplusplus
extern "C" {
#endif

PlayerHandle ivug_player_create();

bool ivug_player_set_file(PlayerHandle hPlayer, const char *szPath);

bool ivug_player_set_mem(PlayerHandle hPlayer, const void *buffer, int len);

bool ivug_player_set_callback(PlayerHandle hPlayer, PlayerCallback cb, void *data);

bool ivug_player_set_config(PlayerHandle hPlayer, PlayerConfig *pConfig);

bool ivug_player_start(PlayerHandle hPlayer);

bool ivug_player_pause(PlayerHandle hPlayer);

bool ivug_player_resume(PlayerHandle hPlayer);

bool ivug_player_stop(PlayerHandle hPlayer);

bool ivug_player_delete(PlayerHandle hPlayer);

bool ivug_player_state_get(PlayerHandle hPlayer, PlayerState * /* INOUT */ state);

#ifdef __cplusplus
}
#endif


#endif	// __IVUG_PLAYER_H__

