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

#ifndef __IVUG_EFFECT_H__
#define __IVUG_EFFECT_H__

#include <stdbool.h>
#include <Elementary.h>
#include "ivug-slideshow.h"
#include "ivug-anim.h"

#define	EFFECT_LBYTES_ARGB8888(w)	(((w) * 4))
#define MAX_PERCENT			(100.0f)
#define MIN_PERCENT			(0.0f)

#ifdef __cplusplus
extern "C" {
#endif

typedef void *Effect_Data ;

typedef void (*FuncFinished)(void *data);

typedef struct {
	Effect_Data eng_data;

	FuncFinished pFinishedCB;
	void *ClientData;

	double t_base;		// start time.
	anim_handle_t *anim;
	Ecore_Animator *animator;

	struct {
		Effect_Data (*init) (Evas_Object *src, Evas_Object *dst);
		bool (*set_size) (Effect_Data data, int screen_w, int screen_h, int rotation);
		void (*animate) (Effect_Data data, double percent, SlideShow *pSlideshow);
		void (*pause) (Effect_Data data);		/* Not used*/
		void (*resume) (Effect_Data data);		/* Not used*/
		void (*finalize) (Effect_Data data);

		double (*get_duration)(Effect_Data data);
	} func;

} Effect_Engine;

typedef enum _Effect_Type {
	EFFECT_NONE,
	EFFECT_SLIDE,
	EFFECT_DISSOLVE_FADE
} Effect_Type;





Effect_Engine *ivug_effect_add(Effect_Type type);

bool ivug_effect_init(Effect_Engine *engine, Evas_Object *src, Evas_Object *dst);
bool ivug_effect_set_size(Effect_Engine *engine, int w, int h, int rotation);
bool ivug_effect_start(Effect_Engine *engine, FuncFinished pFunc, void *data);
bool ivug_effect_pause(Effect_Engine *engine);
bool ivug_effect_resume(Effect_Engine *engine);
bool ivug_effect_finalize(Effect_Engine *engine);


/*
	Private function for adding effect. Do not use below functions directly..
*/
Effect_Engine *ivug_fade_add(void);
Effect_Engine *ivug_slide_add(void);


#ifdef __cplusplus
}
#endif


#endif				/* __IVUG_EFFECT_H__ */
