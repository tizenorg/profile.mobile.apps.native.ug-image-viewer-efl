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

#include "ivug-common.h"
#include "ivug-effect.h"


typedef struct {
	Evas_Object *src;
	Evas_Object *dst;

	Evas_Object *object;
	Evas_Object *orig_cliper;

	Evas_Object *cliper;

	int screen_w;
	int screen_h;
} effect_data;


static Effect_Data __ivug_init(Evas_Object *src, Evas_Object *dst, int screen_w, int screen_h, int rotation)
{
	IVUG_FUNC_ENTER();

	IVUG_FUNC_LEAVE();
}

static void __ivug_anim(Effect_Data data, double percent)
{
/*      IVUG_FUNC_ENTER(); */

/*      IVUG_FUNC_LEAVE(); */
}

static void __ivug_pause(Effect_Data data)
{
	IVUG_FUNC_ENTER();

	IVUG_FUNC_LEAVE();
}

static void __ivug_resume(Effect_Data data)
{
	IVUG_FUNC_ENTER();

	IVUG_FUNC_LEAVE();
}

static void __ivug_spin_finialize(Effect_Data data)
{
	IVUG_FUNC_ENTER();

	IVUG_FUNC_LEAVE();
}

Effect_Engine *ivug_xxx_add(void)
{
	Effect_Engine *eng_spin = calloc(1, sizeof(Effect_Engine));
	ivug_retvm_if(eng_spin == NULL, NULL, "calloc failed");

	eng_spin->func.init = &__ivug_init;
	eng_spin->func.animate = &__ivug_anim;
	eng_spin->func.pause = &__ivug_pause;
	eng_spin->func.resume = &__ivug_resume;
	eng_spin->func.finalize = &__ivug_finialize;

	return eng_spin;
}
