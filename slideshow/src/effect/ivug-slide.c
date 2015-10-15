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
#include <ui-gadget.h>
#include <ui-gadget-module.h>
//#include <Ecore_X.h>


typedef struct {
	Evas_Object *src;
	Evas_Object *dst;

	int screen_w;
	int screen_h;
} Priv_Data;

#define IMAGE_BETWEEN_MARGIN (30)

static Effect_Data __ivug_slide_init(Evas_Object *src, Evas_Object *dst)
{
	Priv_Data *pData = calloc(1, sizeof(Priv_Data));
	IV_ASSERT(pData != NULL);

	pData->src = src;
	pData->dst = dst;

	evas_object_stack_below(dst, src);

	evas_object_move(pData->src, 0, 0);

	return (Effect_Data)pData;
}

static bool __ivug_slide_set_size(void *data, int screen_w, int screen_h, int rotation)
{
	Priv_Data *pData = (Priv_Data *)data;
	IV_ASSERT(pData != NULL);

	pData->screen_w = screen_w;
	pData->screen_h = screen_h;

	return true;
}

static void _ivug_slide_get_screen_dimension(int *width, int *height)
{
	int rotation = elm_win_rotation_get((Evas_Object *)ug_get_window());

	int screen_w = 0;
	int screen_h = 0;
#if 1//Tizen3.0 Build error
	screen_w = 480;
	screen_h = 800;
#else
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &screen_w, &screen_h);
#endif
	if (rotation == 0 || rotation == 180)
	{
		*width = screen_w;
		*height = screen_h;
	}
	else if (rotation == 90 || rotation == 270)
	{
		*width = screen_h;
		*height = screen_w;
	}
}

static void __ivug_slide_anim(Effect_Data data, double percent)
{
	Priv_Data *pData = (Priv_Data *)data;

	int first = 0;
	int last = -pData->screen_w -IMAGE_BETWEEN_MARGIN;
	//MSG_EFFECT_HIGH("pData->screen_w is %d", pData->screen_w);
	//MSG_EFFECT_HIGH("last is %d", last);
	int value = (double)first * (1.0f - percent / 100.0f) + (double)last * (percent / 100.0f);

	//MSG_EFFECT_HIGH("Slide animation. Value=%d %f", value, percent);

	Evas_Coord ow;
	/*
	*  during animation between 2 images, after rotation, 'w' gotten is 0
	*  it's wrong, so let's use _ivug_slide_get_screen_dimension() to update
	*pData->screen_w and pData->screen_h
	*/
	_ivug_slide_get_screen_dimension(&pData->screen_w, &pData->screen_h);

	ow = pData->screen_w;
	//MSG_EFFECT_HIGH("in __ivug_slide_anim, pData->screen_w, pData->screen_h is %d, %d\n", ow, oh);
	//MSG_EFFECT_HIGH("src des pos x is %d, %d\n", value ,oy);
	//MSG_EFFECT_HIGH("dst des pos x is %d, %d\n", value + ow + IMAGE_BETWEEN_MARGIN ,oy);
	evas_object_move(pData->src, value, 0);
	evas_object_move(pData->dst, value + ow + IMAGE_BETWEEN_MARGIN, 0);

}

static void __ivug_slide_pause(Effect_Data data)
{

}

static void __ivug_slide_resume(Effect_Data data)
{

}

static void __ivug_slide_finialize(Effect_Data data)
{
	Priv_Data *pData = (Priv_Data *)data;

	free(pData);

}

static double __ivug_slide_get_duration(Effect_Data data)
{
	return 0.2f;
}



Effect_Engine *ivug_slide_add(void)
{
	Effect_Engine *eng_slide = calloc(1, sizeof(Effect_Engine));
	ivug_retvm_if(eng_slide == NULL, NULL, "calloc failed");

	eng_slide->func.init = __ivug_slide_init;
	eng_slide->func.set_size = __ivug_slide_set_size;
	eng_slide->func.animate = __ivug_slide_anim;
	eng_slide->func.pause = __ivug_slide_pause;
	eng_slide->func.resume = __ivug_slide_resume;
	eng_slide->func.finalize = __ivug_slide_finialize;
	eng_slide->func.get_duration = __ivug_slide_get_duration;

	return eng_slide;
}
