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

#ifndef __IVUG_IMAGE_H__
#define __IVUG_IMAGE_H__

#include <Evas.h>

#include "ivug-define.h"

#ifdef __cplusplus
extern "C" {
#endif


Evas_Object *
ivug_image_create(Evas_Object *parent);


/*
	Load & Unload image file
*/
Evas_Load_Error
ivug_image_file_set(Evas_Object *obj, const char *file, const char *key);

Evas_Load_Error
ivug_image_mem_set(Evas_Object *obj, const char *buffer, unsigned int size, const char *key);

Evas_Load_Error
ivug_image_unset(Evas_Object *obj);

/*
	Configuration
*/

void
ivug_image_hold_set(const Evas_Object *obj, Eina_Bool hold);	// If set HOLD, all events including mouse is ignored.

void
ivug_image_animated_set(const Evas_Object *obj, Eina_Bool bAnimation);		// Determine whether AGIF is animated or not

/*
	Showing
*/
void
ivug_image_zoom_set(Evas_Object *obj, double zoom, Evas_Point *pCenter);

double
ivug_image_zoom_get(const Evas_Object *obj);

void
ivug_image_zoom_reset(Evas_Object *obj, Evas_Point *pCenter);


void
ivug_image_rotate_set(Evas_Object *obj, int degree);

int
ivug_image_rotate_get(Evas_Object *obj);



/*
	Get image's original size.
*/
void
ivug_image_image_size_get(const Evas_Object *obj, int *w, int *h);

Evas_Object *
ivug_image_internal_image_get(Evas_Object *obj);

void
ivug_image_decoded_size_get(const Evas_Object *obj, int *w, int *h);

unsigned char *
ivug_image_decoded_buffer_get(const Evas_Object *obj);


/*
	Get display geometry.
	x,y,w,h is LCD corrdinate
*/
void
ivug_image_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);		// Geometry of internal image

/*
	Crop image and return cropped image as Evas_Object.
	x,y,w,h is LCD corrdinate
*/
Evas_Object *
ivug_image_region_image_get(Evas_Object *obj, int x, int y, int w, int h);


/*

*/
void ivug_image_coordinate_lcd_to_image(Evas_Object *photocam, int lcd_x, int lcd_y, int *img_x, int *img_y);
void ivug_image_coordinate_image_to_lcd(Evas_Object *photocam, int img_x, int img_y, int *lcd_x, int *lcd_y);


/*
	this function do below works.
	  - crop obj in area (x,y,w,h)
	  - resize cropped image upto ow, oh
*/
Evas_Object *
ivug_image_crop_image_get(const Evas_Object *obj, int x, int y, int w, int h, int ow, int oh);


/*
	Autofit paste to frame whose dimension is frame_w/h
*/
Evas_Object *
ivug_image_frame_image_get(const Evas_Object *obj, int frame_w, int frame_h);


#ifdef __cplusplus
}
#endif




#endif		// __IVUG_IMAGE_H__

