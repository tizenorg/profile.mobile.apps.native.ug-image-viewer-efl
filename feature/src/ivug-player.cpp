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
#include "ivug-player.h"
#include "ivug-debug.h"

#include <player.h>

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-PLAYER"

#undef USE_STARTED_CB

struct CPlayer {
public:
	CPlayer() : pPlayerHandle(NULL), bUseSurface(false), UserCB(NULL), UserData(NULL)	{ };

	player_h pPlayerHandle;		// Player handle

	bool bUseSurface;

// User callback
	PlayerCallback UserCB;
	void *UserData;

	operator PlayerHandle() const { return (PlayerHandle)this; };
};

static const char *_strerror_player(int error)
{
	switch(error)
	{
	case PLAYER_ERROR_OUT_OF_MEMORY :
		return "Out of memory";
	case PLAYER_ERROR_INVALID_PARAMETER :
		return "Invalid parameter";
	case PLAYER_ERROR_NO_SUCH_FILE :
		return "No such file or directory";
	case PLAYER_ERROR_INVALID_OPERATION :
		return "Invalid operation";
	case PLAYER_ERROR_SEEK_FAILED :
		return "Seek operation failure";
	case PLAYER_ERROR_INVALID_STATE :
		return "Invalid state";
	case PLAYER_ERROR_NOT_SUPPORTED_FILE :
		return "Not supported file format";
	case PLAYER_ERROR_INVALID_URI :
		return "Invalid URI";
	case PLAYER_ERROR_SOUND_POLICY :
		return "Sound policy error";
	case PLAYER_ERROR_CONNECTION_FAILED :
		return "Streaming connection failed";
	case PLAYER_ERROR_VIDEO_CAPTURE_FAILED :
		return "Video capture failure";
	default:
		{
			static char buf[40];
			snprintf(buf, 40, "Error Code=%d", error);
			return buf;
		}

	}
}

static const char *_str_player_state_e(player_state_e state)
{
/*
PLAYER_STATE_NONE  Player is not created
PLAYER_STATE_IDLE  Player is created, but not prepared
PLAYER_STATE_READY  Player is ready to play media
PLAYER_STATE_PLAYING  Player is playing media
PLAYER_STATE_PAUSED  Player is paused while playing media

*/
	switch(state)
	{
	case PLAYER_STATE_NONE:
		return "Player is not created";
	case PLAYER_STATE_IDLE:
		return "Player is created, but not prepared ";
	case PLAYER_STATE_READY:
		return "Player is ready to play media";
	case PLAYER_STATE_PLAYING:
		return "Player is playing media";
	case PLAYER_STATE_PAUSED:
		return "Player is paused while playing media";

	default:
		{
			static char buf[40];
			snprintf(buf, 40, "Unknown state=%d", state);
			return buf;
		}
	}
}


static void _OnPlayerError(int error_code, void *user_data)
{
	MSG_HIGH("On Player Error(%d) %s", error_code, _strerror_player(error_code));

	CPlayer *pPlayer = static_cast<CPlayer *>(user_data);

	if (pPlayer->UserCB )
	{
		PlayerParam param;

		param.type = IVUG_PLAYER_EVENT_ERROR;
		param.param.error.error_code = error_code;

		(pPlayer->UserCB)(&param, pPlayer->UserData);

	}
}

static void _OnPlayerInterrupted(player_interrupted_code_e code, void *user_data)
{
	MSG_HIGH("On Player Interrupted : %d", code);
}

static void _OnPlayerEOS(void *user_data)
{
	MSG_HIGH("On Player EOS");

	CPlayer *pPlayer = static_cast<CPlayer *>(user_data);

	int nErr = PLAYER_ERROR_NONE;

	nErr = player_stop(pPlayer->pPlayerHandle);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Player Stop Error(%d) %s", nErr, _strerror_player(nErr));
		// Go through
	}

	nErr = player_unprepare(pPlayer->pPlayerHandle);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Player Unprepare Error(%d) %s", nErr, _strerror_player(nErr));
		// Go through
	}

	if (pPlayer->UserCB )
	{
		PlayerParam param;

		param.type = IVUG_PLAYER_EVENT_EOS;

		(pPlayer->UserCB)(&param, pPlayer->UserData);
	}
}

#ifdef USE_PREPARE_CB
static void _OnPlayerPrepared(void *user_data)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(user_data);

	MSG_HIGH("On Player Prepared");

	if (pPlayer->UserCB )
	{
		PlayerParam param;

		param.type = IVUG_PLAYER_EVENT_PREPARED;

		(pPlayer->UserCB)(&param, pPlayer->UserData);
	}
}
#endif

static bool register_callback(CPlayer *pPlayer)
{
	int ret = PLAYER_ERROR_NONE;

	ret = player_set_completed_cb(pPlayer->pPlayerHandle, _OnPlayerEOS, pPlayer);
	if(ret != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Cannot set callback : %s", _strerror_player(ret));
		return false;
	}

	ret = player_set_interrupted_cb(pPlayer->pPlayerHandle, _OnPlayerInterrupted, pPlayer);
	if(ret != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Cannot set callback : %s", _strerror_player(ret));
		return false;
	}

	ret = player_set_error_cb(pPlayer->pPlayerHandle, _OnPlayerError, pPlayer);
	if( ret != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Cannot set callback : %s", _strerror_player(ret));
		return false;
	}

	return true;
}



PlayerHandle ivug_player_create()
{
	CPlayer *pPlayer = new CPlayer;

	sound_manager_set_session_type(SOUND_SESSION_TYPE_MEDIA);
	int nErr = PLAYER_ERROR_NONE;

	try {
		nErr = player_create(&pPlayer->pPlayerHandle);
		if(nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("Fail to create player handle. (MMF Error code : %x)", nErr);
			throw 
"create fail";
		}

		register_callback(pPlayer);

	}
	catch(...)
	{
		MSG_ERROR("Creation player failed");

		delete pPlayer;

		return NULL;
	}

	MSG_HIGH("Create player Handle=0x%08x", pPlayer);
	return (PlayerHandle)pPlayer;

}


bool ivug_player_set_file(PlayerHandle hPlayer, const char *szPath)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	nErr = player_set_uri(pPlayer->pPlayerHandle, szPath);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("(%x) Fail to set uri", nErr);
		return false;
	}

	nErr = player_set_sound_type(pPlayer->pPlayerHandle, SOUND_TYPE_MEDIA);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("(%x):: Fail to set attribute ", nErr);
		return false;
	}

	MSG_HIGH("Set file : %s", szPath);
	return true;
}

bool ivug_player_set_mem(PlayerHandle hPlayer, const void *buffer, int len)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	nErr = player_set_memory_buffer(pPlayer->pPlayerHandle, buffer, len);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("(%x) Fail to set uri", nErr);
		return false;
	}

	nErr = player_set_sound_type(pPlayer->pPlayerHandle, SOUND_TYPE_MEDIA);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("(%x):: Fail to set attribute ", nErr);
		return false;
	}

	MSG_HIGH("Set Mem : Buffer=0x%08x Len=%d", buffer, len);
	return true;

}


bool ivug_player_set_config(PlayerHandle hPlayer, PlayerConfig *pConfig)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	if (pConfig == NULL)
	{
		MSG_WARN("No player config data. set default.");
		return true;
	}

	int nErr = PLAYER_ERROR_NONE;

	if (pConfig->bLooping == true )
	{
		nErr = player_set_looping(pPlayer->pPlayerHandle, true);

		if(nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("[ERR] Error code : %x - Fail to set looping ", nErr);
			return false;
		}
	}
	else
	{
		nErr = player_set_looping(pPlayer->pPlayerHandle, false);

		if(nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("[ERR] Error code : %x - Fail to set looping ", nErr);
			return false;
		}
	}

	if (pConfig->eSurface == IVUG_PLAYER_SURFACE_EVAS)
	{
		if (pConfig->pSurface == NULL)
		{
			MSG_FATAL("Invalid surface.");

			return false;
		}

		nErr = player_set_display(pPlayer->pPlayerHandle, PLAYER_DISPLAY_TYPE_EVAS, pConfig->pSurface);
		if(nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("[ERR] Error code : %x - Fail to set display ", nErr);
			return false;
		}

//		nErr = player_enable_evas_display_scaling(pPlayer->pPlayerHandle, false);
//		if( nErr != PLAYER_ERROR_NONE)
//		{
//			MSG_ERROR("[ERR] Error code : %x - Fail to set evas_display_scaling ", nErr);
//			return false;
//		}

		pPlayer->bUseSurface = true;

	}
	else if (pConfig->eSurface == IVUG_PLAYER_SURFACE_X11)
	{
		if (pConfig->pSurface == NULL)
		{
			MSG_FATAL("Invalid surface.");

			return false;
		}

		nErr = player_set_display_mode(pPlayer->pPlayerHandle, PLAYER_DISPLAY_MODE_FULL_SCREEN);
		if (nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("[ERR] Error code : %x - Fail to set display_mpde ", nErr);
		}

		nErr = player_set_display(pPlayer->pPlayerHandle, PLAYER_DISPLAY_TYPE_OVERLAY, (void*)pConfig->pSurface);

		if( nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("[ERR] Error code : %x - Fail to set display", nErr);

			return false;
		}

//		nErr = player_enable_evas_display_scaling(pPlayer->pPlayerHandle, true);
//		if (nErr != PLAYER_ERROR_NONE)
//		{
//			MSG_ERROR("[ERR] Error code : %x - Fail to set evas_display_scaling", nErr);
//		}

		nErr = player_set_display_visible(pPlayer->pPlayerHandle, true);
		if (nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("[ERR] Error code : %x - Fail to set x11_display_visible", nErr);
		}

		pPlayer->bUseSurface = true;
	}
	else
	{
		pPlayer->bUseSurface = false;

		MSG_WARN("No surface is specified.");
	}

	MSG_HIGH("Set Config : Loop(%d) eSurface(%d) Surface(0x%08x)", pConfig->bLooping, pConfig->eSurface, pConfig->pSurface);

	return true;
}

bool ivug_player_set_callback(PlayerHandle hPlayer, PlayerCallback cb, void *data)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	pPlayer->UserCB = cb;
	pPlayer->UserData = data;

	return true;
}


bool ivug_player_start(PlayerHandle hPlayer)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	player_state_e pstate;

	nErr = player_get_state(pPlayer->pPlayerHandle, &pstate);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Player Get state Error(%d) %s", nErr, _strerror_player(nErr));
		return false;
	}

	if (pstate != PLAYER_STATE_PAUSED)
	{
		MSG_HIGH("Player Prepare");

		nErr = player_prepare(pPlayer->pPlayerHandle);
		if(nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("Error code : 0x%x %s", nErr, _strerror_player(nErr));
			return false;
		}
	}

	MSG_HIGH("Player Start");

	nErr = player_start(pPlayer->pPlayerHandle);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Error code : 0x%x", nErr);
		return false;
	}

	int w = 0, h = 0;

	if (pPlayer->bUseSurface == true )
	{
		nErr = player_get_video_size(pPlayer->pPlayerHandle, &w, &h);

		MSG_HIGH("Player Start. Stream(%d,%d)", w, h);

		if( nErr != PLAYER_ERROR_NONE)
		{
			MSG_ERROR("Cannot get vide size : %s", _strerror_player(nErr));
		}
	}

	MSG_HIGH("Player Started");

	if (pPlayer->UserCB )
	{
		PlayerParam param;

		param.type = IVUG_PLAYER_EVENT_STARTED;

// In case of audio only, w&h is 0
		param.param.started.w = w;
		param.param.started.h = h;

		(pPlayer->UserCB)(&param, pPlayer->UserData);
	}

	return true;
}


bool ivug_player_pause(PlayerHandle hPlayer)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	nErr = player_pause(pPlayer->pPlayerHandle);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Error code : 0x%x", nErr);
		return false;
	}

	MSG_HIGH("Player pause");

	if (pPlayer->UserCB )
	{
		PlayerParam param;

		param.type = IVUG_PLAYER_EVENT_PAUSED;

		(pPlayer->UserCB)(&param, pPlayer->UserData);
	}

	return true;
}


bool ivug_player_resume(PlayerHandle hPlayer)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	nErr = player_start(pPlayer->pPlayerHandle);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Error code : 0x%x", nErr);
		return false;
	}

	MSG_HIGH("Player resume");

	return true;

}


bool ivug_player_stop(PlayerHandle hPlayer)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	player_state_e pstate;

	nErr = player_get_state(pPlayer->pPlayerHandle, &pstate);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Player Get state Error(%d) %s", nErr, _strerror_player(nErr));
		return false;
	}

	if (pstate == PLAYER_STATE_PAUSED || pstate == PLAYER_STATE_PLAYING)
	{
		MSG_HIGH("Player_Stop");

		nErr = player_stop(pPlayer->pPlayerHandle);
		if(nErr != PLAYER_ERROR_NONE)
		{
			int err = PLAYER_ERROR_NONE;
			player_state_e pstate;

			err = player_get_state(pPlayer->pPlayerHandle, &pstate);
			if(err != PLAYER_ERROR_NONE)
			{
				MSG_ERROR("Player Get state Error(%d) %s", nErr, _strerror_player(nErr));
				return false;
			}

			MSG_HIGH("Player Stop Error(%d) %s. State=%s", nErr, _strerror_player(nErr), _str_player_state_e(pstate) );
			return false;
		}

	}

	nErr = player_unprepare(pPlayer->pPlayerHandle);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_HIGH("Player Unprepare Error(%d) %s", nErr, _strerror_player(nErr));

		return false;
	}

	MSG_HIGH("Player stop");

	return true;

}



bool ivug_player_delete(PlayerHandle hPlayer)
{
	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	nErr = player_destroy(pPlayer->pPlayerHandle);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_ERROR("Error code : 0x%x", nErr);
// Go through
	}

	MSG_HIGH("Player destroy. Handle=0x%08x", pPlayer);

	sound_manager_set_session_type(SOUND_SESSION_TYPE_MEDIA);

	delete pPlayer;

	return true;
}

bool ivug_player_state_get(PlayerHandle hPlayer, PlayerState * /* INOUT */ state)
{
	MSG_ASSERT(state != NULL);

	CPlayer *pPlayer = static_cast<CPlayer *>(hPlayer);

	int nErr = PLAYER_ERROR_NONE;

	player_state_e pstate;

	nErr = player_get_state(pPlayer->pPlayerHandle, &pstate);
	if(nErr != PLAYER_ERROR_NONE)
	{
		MSG_HIGH("Player Get state Error(%d) %s", nErr, _strerror_player(nErr));
		*state = IVUG_PLAYER_STATE_NONE;
		return false;
	}

	switch(pstate)
	{
	case PLAYER_STATE_NONE:
	case PLAYER_STATE_IDLE:
	case PLAYER_STATE_READY:
		*state = IVUG_PLAYER_STATE_CREATED;
		break;
	case PLAYER_STATE_PLAYING:
		*state = IVUG_PLAYER_STATE_PLAYING;
		break;
	case PLAYER_STATE_PAUSED:
		*state = IVUG_PLAYER_STATE_PAUSED;
		break;
	default:
		MSG_FATAL("Invalid pstate : %d", pstate);
		*state = IVUG_PLAYER_STATE_NONE;
		break;
	}

	return true;
}



