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

typedef struct _Fade_Data Fade_Data;

struct _Fade_Data {
	Evas_Object *src;
	Evas_Object *dst;

	Evas_Object *object;
	Evas_Object *orig_cliper;

	Evas_Object *cliper;

	Evas * evas;

	int screen_w;
	int screen_h;
};

static void *__ivug_fade_init(Evas_Object *src, Evas_Object *dst)
{
	IVUG_FUNC_ENTER();

	Fade_Data *eng_data = calloc(1, sizeof(Fade_Data));
	ivug_retvm_if(eng_data == NULL, NULL, "calloc return NULL");

	eng_data->evas = evas_object_evas_get(src);

	eng_data->src = src;
	eng_data->dst = dst;

// Move object to 0,0
	evas_object_stack_below(dst, src);

	evas_object_move(dst, 0, 0);
	evas_object_move(src, 0, 0);

	Evas_Object *orig_cliper = evas_object_clip_get(src);
	eng_data->orig_cliper = orig_cliper;

	IVUG_FUNC_LEAVE();

	return (Effect_Data)eng_data;
}

static bool __ivug_fade_set_size(void *data, int screen_w, int screen_h, int rotation)
{
	IVUG_FUNC_ENTER();

	ivug_retvm_if(data == NULL, false, "data is NULL");

	Fade_Data *eng_data = (Fade_Data *) data;

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(eng_data->src, &x, &y, &w, &h);

	eng_data->screen_w = screen_w;
	eng_data->screen_h = screen_h;

	if (eng_data->cliper) {
		evas_object_clip_unset(eng_data->src);
		evas_object_del(eng_data->cliper);
		eng_data->cliper = NULL;
	}

	Evas_Object *clip = evas_object_rectangle_add(eng_data->evas);
	evas_object_color_set(clip, 255, 255, 255, 255);
	evas_object_resize(clip, screen_w, screen_h);
	evas_object_move(clip, x, y);
	evas_object_show(clip);

	evas_object_clip_set(eng_data->src, clip);
	eng_data->cliper = clip;

	evas_object_move(eng_data->dst, 0, 0);

	IVUG_FUNC_LEAVE();

	return true;
}

static void __ivug_fade_anim(void *data, double percent)
{
	Fade_Data *eng_data = (Fade_Data *) data;

	ivug_retm_if(eng_data == NULL, "data is NULL");
	ivug_retm_if(eng_data->cliper == NULL, "cliper is NULL");

	int alpha = 255 * (MAX_PERCENT - percent) / MAX_PERCENT;
	evas_object_color_set(eng_data->cliper, alpha, alpha, alpha, alpha);

	MSG_EFFECT_MED("alpha = %d", alpha);
}

static void __ivug_fade_pause(void *data)
{
	MSG_EFFECT_HIGH("Fade paused");
}

static void __ivug_fade_resume(void *data)
{
	MSG_EFFECT_HIGH("Fade resumed");
}

static void __ivug_fade_finialize(void *data)
{
	MSG_EFFECT_HIGH("Fade destroy");

	ivug_retm_if(data == NULL, "data is NULL");

	Fade_Data *eng_data = (Fade_Data *) data;

	if (eng_data->cliper) {
		evas_object_clip_unset(eng_data->src);
		evas_object_del(eng_data->cliper);
		eng_data->cliper = NULL;
	}

	if (eng_data->src && eng_data->orig_cliper)
	{
		evas_object_clip_set(eng_data->src, eng_data->orig_cliper);
	}

	free(eng_data);
	eng_data = NULL;

}

Effect_Engine *ivug_fade_add(void)
{
	Effect_Engine *eng_fade = calloc(1, sizeof(Effect_Engine));
	ivug_retvm_if(eng_fade == NULL, NULL, "calloc failed");

	eng_fade->func.init = &__ivug_fade_init;
	eng_fade->func.set_size = &__ivug_fade_set_size;
	eng_fade->func.animate = (void *)(&__ivug_fade_anim);
	eng_fade->func.pause = &__ivug_fade_pause;
	eng_fade->func.resume = &__ivug_fade_resume;
	eng_fade->func.finalize = &__ivug_fade_finialize;

	return eng_fade;
}

