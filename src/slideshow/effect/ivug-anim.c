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


#include <stddef.h>		/* for NULL */
#include <memory.h>
#include <stdlib.h>

#include "ivug-debug.h"
#include "ivug-anim.h"

/*
	t: current time b: start time c: change in value d: duration
*/
static double _anim_linear(double t, double b, double c, double d)
{
	if (d != 0.0)
		t /= d;
	return b + c * (t);
}

static double _anim_sine(double t, double b, double c, double d)
{
/* TODO : Consider below table reconstruct. need only 0.0 ~ 1.0 range*/
	static const double SIN_TABLE[91] = {
		0.0000f, 0.0174f, 0.0349f, 0.0523f, 0.0698f,
		0.0872f, 0.1045f, 0.1219f, 0.1392f, 0.1564f,
		0.1736f, 0.1908f, 0.2079f, 0.2249f, 0.2419f,
		0.2588f, 0.2756f, 0.2924f, 0.3090f, 0.3256f,
		0.3420f, 0.3584f, 0.3746f, 0.3907f, 0.4067f,
		0.4226f, 0.4384f, 0.4540f, 0.4695f, 0.4848f,
		0.5000f, 0.5150f, 0.5299f, 0.5446f, 0.5592f,
		0.5736f, 0.5878f, 0.6018f, 0.6157f, 0.6293f,
		0.6528f, 0.6561f, 0.6691f, 0.6820f, 0.6947f,
		0.7071f, 0.7193f, 0.7314f, 0.7431f, 0.7547f,
		0.7660f, 0.7772f, 0.7880f, 0.7986f, 0.8090f,
		0.8191f, 0.8290f, 0.8387f, 0.8480f, 0.8571f,
		0.8660f, 0.8746f, 0.8829f, 0.8910f, 0.8988f,
		0.9063f, 0.9135f, 0.9205f, 0.9272f, 0.9336f,
		0.9397f, 0.9455f, 0.9511f, 0.9563f, 0.9613f,
		0.9659f, 0.9703f, 0.9744f, 0.9781f, 0.9816f,
		0.9848f, 0.9877f, 0.9903f, 0.9926f, 0.9945f,
		0.9962f, 0.9976f, 0.9986f, 0.9994f, 0.9998f,
		1.0f
	};

	if (d != 0.0)
		t /= d;			/* normalize */

	int idx = (int)(90.0 * t);

	return b + c * (SIN_TABLE[idx]);

}

static double _anim_ease_inout_quartic(double t, double b,
						double c, double d)
{
	if (d != 0.0)
		t /= d;
	double ts = t * t;
	double tc = ts * t;
	return b + c * (-2 * tc + 3 * ts);

}

static double _anim_ease_inout_quintic(double t, double b,
						double c, double d)
{
	if (d != 0.0)
		t /= d;
	double ts = t * t;
	double tc = ts * t;
	return b + c * (6 * tc * ts + -15 * ts * ts + 10 * tc);
}

anim_handle_t *create_animation()
{
	anim_handle_t *anim;

	anim = calloc(1, sizeof(anim_handle_t));

	if (anim == NULL) {
		MSG_IVUG_ERROR("Cannot allocate memory");
		return NULL;
	}

	return anim;

}

void delete_animation(anim_handle_t *anim)
{
	IV_ASSERT(anim != NULL);

	free(anim);

}

bool set_animation_type(anim_handle_t *anim, anim_type_t type)
{
	IV_ASSERT(anim != NULL);

	switch (type) {
	case ANIM_TYPE_LINEAR:
		anim->transit_func = _anim_linear;
		break;
	case ANIM_TYPE_QUARTIC_INOUT:
		anim->transit_func = _anim_ease_inout_quartic;
		break;

	case ANIM_TYPE_QUINTIC_INOUT:
		anim->transit_func = _anim_ease_inout_quintic;
		break;

	case ANIM_TYPE_SINE:
		anim->transit_func = _anim_sine;
		break;
	default:
		MSG_IVUG_ERROR("Invalid transition type=%d", type);
		return false;
		break;
	}

	return true;
}

bool set_animation(anim_handle_t *anim, int start, int end, double duration)
{
	IV_ASSERT(anim != NULL);

	//MSG_IVUG_HIGH("Set Anim. Start=%d End=%d Dur=%f", start, end, duration);
	anim->duration = duration;
	anim->t_start = start;
	anim->t_varing = end - start;

	return true;
}

bool update_animation(anim_handle_t *anim, double current, double *value)
{
	//MSG_IVUG_HIGH("update_animation");
	IV_ASSERT(anim != NULL);
	IV_ASSERT(anim->transit_func != NULL);

	if (current >= anim->duration) {
		*value = anim->t_start + anim->t_varing;
		//MSG_IVUG_HIGH("End Start:%f Varing:%f Duration=%f",
			     //anim->t_start, anim->t_varing, anim->duration);

		//MSG_IVUG_HIGH("End Current=%f Value=%f", current, *value);

		return false;	/* End of animation */
	}

	MSG_EFFECT_LOW("Start:%f Varing:%f Duration=%f Current=%f", anim->t_start, anim->t_varing, anim->duration, current);

	*value = anim->transit_func(current, anim->t_start, anim->t_varing,
					 anim->duration);

	MSG_EFFECT_LOW("Value = %f", *value);

	return true;
}
