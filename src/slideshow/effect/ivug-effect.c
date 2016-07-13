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

#include "ivug-debug.h"
#include "ivug-effect.h"

#define SLIDESHOW_ANI_TIME (0.7f)


static Eina_Bool
_on_effect_animator(void *data)
{
	IV_ASSERT(data != NULL);
	Effect_Engine *engine = data;

	double value = 0.0;

	if (update_animation(engine->anim, ecore_loop_time_get() - engine->t_base, &value) == false) 	 // If last frame
	{
		engine->animator = NULL;

		MSG_EFFECT_HIGH("Last effect frame. value=%f", value);

		engine->func.animate(engine->eng_data, value, (SlideShow*)(engine->ClientData));		// Draw last frame.
		// Inform to user.
		if (engine->pFinishedCB)
		{
			engine->pFinishedCB(engine->ClientData);
		}

		return ECORE_CALLBACK_CANCEL;
	}

	engine->func.animate(engine->eng_data, value, (SlideShow*)(engine->ClientData));

	return ECORE_CALLBACK_RENEW;
}


bool ivug_effect_init(Effect_Engine *engine, Evas_Object *src, Evas_Object *dst)
{
	IV_ASSERT(engine != NULL);

	void *data = engine->func.init(src, dst);
	if (data == NULL) {
		MSG_EFFECT_ERROR("init returned data is NULL");

		return false;
	}

	engine->eng_data = data;

	engine->anim = create_animation();
	return true;
}

bool ivug_effect_set_size(Effect_Engine *engine, int w, int h, int rotation)
{
	IV_ASSERT(engine != NULL);
	IV_ASSERT(engine->eng_data != NULL);

	if (engine->func.set_size == NULL)
	{
		MSG_EFFECT_ERROR("engine->func.set_size is NULL");
		return false;
	}

	MSG_EFFECT_HIGH("set size. WH(%d,%d) Rotation(%d)", w, h, rotation);
	return engine->func.set_size(engine->eng_data, w, h, rotation);
}


bool ivug_effect_start(Effect_Engine *engine, FuncFinished pFunc, void *data)
{
	ivug_retvm_if(engine == NULL, false, "engine is NULL");

	IV_ASSERT(engine->func.animate != NULL);
	IV_ASSERT(engine->eng_data != NULL);

// Set up slide show animator
	engine->t_base = ecore_loop_time_get();

	double duration = SLIDESHOW_ANI_TIME;

	if (engine->func.get_duration)
	{
		duration = engine->func.get_duration(engine->eng_data);
	}

	set_animation_type(engine->anim, ANIM_TYPE_LINEAR);
	set_animation(engine->anim, 0, 100, duration);	// 0~100% during 1 seconds

	engine->pFinishedCB = pFunc;
	engine->ClientData = data;

	if (engine->animator == NULL)
	{
		engine->animator = ecore_animator_add(_on_effect_animator, engine);
	}

	return true;
}

bool ivug_effect_pause(Effect_Engine *engine)
{
/* Unused. */
	return true;
}

bool ivug_effect_resume(Effect_Engine *engine)
{
/* Unused. */
	return true;
}

bool ivug_effect_finalize(Effect_Engine *engine)
{
	ivug_retvm_if(engine == NULL, false, "engine is NULL");

	if (engine->animator)
	{
		ecore_animator_del(engine->animator);
		engine->animator = NULL;
	}

	if (engine->eng_data)
	{
		if (engine->func.finalize)
		{
			MSG_EFFECT_HIGH("finalize");
			engine->func.finalize(engine->eng_data);
		}
		else
		{
			MSG_EFFECT_ERROR("engine->func.finalize is NULL");
		}

		engine->eng_data = NULL;
	}
	else
	{
		MSG_EFFECT_ERROR("engine->eng_data is NULL");
	}

	if (engine->anim)
	{
		delete_animation(engine->anim);
	}

	free(engine);
	engine = NULL;

	return true;
}

Effect_Engine *ivug_effect_add(Effect_Type type)
{
	Effect_Engine *eng = NULL;

	switch (type) {
	case EFFECT_DISSOLVE_FADE:
		eng = ivug_fade_add();
		break;

	case EFFECT_SLIDE:
		eng = ivug_slide_add();
		break;

	default:
		MSG_EFFECT_ERROR("Unknown effect type=%d", type);
		return NULL;
		break;
	}

	if (eng == NULL) {
		MSG_EFFECT_ERROR("Cannot create effect enigne. type=%d", type);
		return NULL;
	}

	return eng;
}
