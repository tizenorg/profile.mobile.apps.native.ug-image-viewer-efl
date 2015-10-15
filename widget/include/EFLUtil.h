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

#pragma once

#include <Elementary.h>

namespace EFL {
	void get_evas_geometry(Evas_Object *obj, const char *prefix);

	Evas_Object *create_gesture(Evas_Object *parent);

	Evas_Object *create_rect(Evas_Object *parent, int r, int g, int b, int a);

	Evas_Object *create_clipper(Evas_Object *parent);

	Evas_Object *create_layout(Evas_Object *parent, const char *edj, const char *group);

	Evas_Object *create_button(Evas_Object *parent, const char *style, const char *icon, const char *caption);

	Evas_Object *create_icon(Evas_Object *parent, const char *edjname, const char *groupname);

	void dump_win(Evas_Object *obj);

	void dump_obj(Evas_Object *obj, int lvl = 0);
	void dump_widget(Evas_Object *obj, int lvl);

	void dump_clipper(Evas_Object *clipper);
	// Returned value Should be freed after using
	char * current_render_method(Evas *e);

	Evas_Object *create_window(const char *title, int w, int h);

};

namespace util {
	typedef enum {
		eImageCodec_JPEG,
		eImageCodec_GIF,
		eImageCodec_PNG,
		eImageCodec_IMAGE,			// tif.. other image files
		eImageCodec_UNKNOWN,
	} Image_Codec_Type;

	Image_Codec_Type
	get_codec_type_with_size(const unsigned char *buffer, unsigned int size);

	Image_Codec_Type
	get_codec_type(const char *filename);

	bool is_openGL_enabled(Evas *e);
};


