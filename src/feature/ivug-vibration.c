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

#include "ivug-vibration.h"
#include "ivug-debug.h"

#include <stdio.h>
#ifdef USE_MAXLENGTH_VIBE
#include <dd-haptic.h>

static const char *_conver_error(int err)
{
	switch (err)
	{
		case HAPTIC_ERROR_NONE:
			return "Successful";
		case HAPTIC_ERROR_INVALID_PARAMETER:
			return "Invalid parameter";
		case HAPTIC_ERROR_NOT_INITIALIZED:
			return "Not initialized";
		case HAPTIC_ERROR_OPERATION_FAILED:
			return "Operation failed";
		case HAPTIC_ERROR_NOT_SUPPORTED_DEVICE:
			return "Not supported device";
		default:
		{
			static char error[128];
			snprintf(error, 128, "Unknow Error : %d(0x%08x)", err, err);
			return error;
		}
	}

	return NULL;

}


vibration_h ivug_vibration_create()
{
	int ret = HAPTIC_ERROR_NONE;
	haptic_device_h handle = NULL;

	ret = haptic_open(HAPTIC_DEVICE_ALL, &handle);

	if (ret != HAPTIC_ERROR_NONE)
	{
		MSG_UTIL_ERROR("device_haptic_open failed. %s", _conver_error(ret));
		return NULL;
	}

	MSG_UTIL_HIGH("Vibration init. Handle=%x", handle);

	return (vibration_h)handle;
}

bool ivug_vibration_play(vibration_h v_handle, int duration)
{
	IV_ASSERT(v_handle!=NULL);
	int ret = HAPTIC_ERROR_NONE;
	haptic_device_h handle = (haptic_device_h)v_handle;

	MSG_UTIL_HIGH("Vibration start. Handle=%x duration=%d", handle, duration);

	ret = haptic_vibrate_monotone(handle, duration, NULL);

	if (ret != HAPTIC_ERROR_NONE)
	{
		MSG_UTIL_ERROR("haptic_vibrate_monotone error Handle=%x. %s", handle, _conver_error(ret));
		return false;
	}

	return true;
}

bool ivug_vibration_stop(vibration_h v_handle)
{
	IV_ASSERT(v_handle!=NULL);
	int ret = HAPTIC_ERROR_NONE;
	haptic_device_h handle = (haptic_device_h)v_handle;

	MSG_UTIL_HIGH("Vibration stop. Handle=%x", handle);

	ret = haptic_stop_all_effects(handle);
	if (ret != 0)
	{
		MSG_UTIL_ERROR("haptic_stop_device failed. %s",  _conver_error(ret));
	}

	return true;
}


bool ivug_vibration_delete(vibration_h v_handle)
{
	IV_ASSERT(v_handle!=NULL);
	int ret = HAPTIC_ERROR_NONE;
	haptic_device_h handle = (haptic_device_h)v_handle;

	MSG_UTIL_HIGH("Vibration deinit. Handle=%x", handle);

	ret = haptic_close(handle);

	if (ret != HAPTIC_ERROR_NONE)
	{
		MSG_UTIL_ERROR("device_haptic_close failed. %s", _conver_error(ret));
	}

	return true;
}
#endif

