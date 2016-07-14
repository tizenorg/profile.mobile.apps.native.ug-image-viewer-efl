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

#include <Elementary.h>

#include "ivug-crop-ug.h"
#include "ivug-crop-view.h"

#include "ivug-debug.h"
#include "ivug-string.h"
#include "ivug-context.h"
#include "ivug-db.h"

#include "ivug-language-mgr.h"
#include "ivug-common.h"

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_MED

#undef LOG_CAT
#define LOG_CAT "IV-CROP-UG"



/*
Setting->Wallpaper setting(UG)
	W/IV-COMMON( 4050): 0:00:00.003[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 4050): 0:00:00.003[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/image9.jpg
	W/IV-COMMON( 4050): 0:00:00.003[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 4050): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Setas type : Wallpaper
	W/IV-COMMON( 4050): 0:00:00.004[F:ivug-parameter.c L:  446][HIGH] **********************************

Setting -> Lockscreen setting(UG)
	W/IV-COMMON( 4050): 0:02:42.425[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 4050): 0:02:42.425[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/Home_default.jpg
	W/IV-COMMON( 4050): 0:02:42.425[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 4050): 0:02:42.426[F:ivug-parameter.c L:  370][HIGH]   Setas type : Lockscreen
	W/IV-COMMON( 4050): 0:02:42.426[F:ivug-parameter.c L:  446][HIGH] **********************************

Contacts -> CallerID(UG)
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Downloads/aaa_1_1_1.jpg
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  370][HIGH]   Setas type : CallerID
	W/IV-COMMON( 7761): 0:00:00.011[F:ivug-parameter.c L:  370][HIGH]   Area Size : 100
	W/IV-COMMON( 7761): 0:00:00.011[F:ivug-parameter.c L:  370][HIGH]   Resolution : 480x480
	W/IV-COMMON( 7761): 0:00:00.011[F:ivug-parameter.c L:  446][HIGH] **********************************

Long Press in homescreen -> Menu -> Change wallpaper
	W/IV-COMMON( 7229): 0:00:00.012[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 7229): 0:00:00.013[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 7229): 0:00:00.013[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/image2.jpg
	W/IV-COMMON( 7229): 0:00:00.014[F:ivug-parameter.c L:  370][HIGH]   Setas type : Wallpaper Crop
	W/IV-COMMON( 7229): 0:00:00.014[F:ivug-parameter.c L:  370][HIGH]   Fixed ratio : TRUE
	W/IV-COMMON( 7229): 0:00:00.014[F:ivug-parameter.c L:  370][HIGH]   Resolution : 720x1280
	W/IV-COMMON( 7229): 0:00:00.015[F:ivug-parameter.c L:  446][HIGH] **********************************

Gallery -> Image Viewer -> Menu -> Print
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/Home_default.jpg
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Setas type : Crop
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Fixed ratio : TRUE
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Resolution : 450x300
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  446][HIGH] **********************************

SMEMO -> Add image

	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Camera/20130525-193717.jpg
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  370][HIGH]   Setas type : Crop
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  446][HIGH] **********************************

--> On Save��
	W/IV-CROP-UG( 7925): 0:00:33.557[F:ivug-crop-ug.cpp L:   94][HIGH] Bundle 1 : [crop_image_path = /opt/usr/media/Images/image2_1.jpg]
	W/IV-CROP-UG( 7925): 0:00:33.557[F:ivug-crop-ug.cpp L:   99][HIGH] Bundle 2 : [http://tizen.org/appcontrol/data/selected = /opt/usr/media/Images/image2_1.jpg]
*/


struct _IvugCropUG {

	Evas_Object *layout;
	Evas_Object *parent;
	char *filepath;
	bool bAddtoDB;
} ;

Evas_Object * ivug_crop_ug_get_layout(IvugCropUG * crop_ug)
{
	IV_ASSERT(crop_ug != NULL);

	return crop_ug->layout;
}

bool ivug_crop_ug_destroy(IvugCropUG * crop_ug)
{
	IV_ASSERT(crop_ug != NULL);

	if (crop_ug->filepath) {
		free(crop_ug->filepath);
		crop_ug->filepath = NULL;
	}

	crop_ug->layout = NULL;

	free(crop_ug);

	return true;
}

bool ivug_crop_ug_start(IvugCropUG * crop_ug)
{
	MSG_HIGH("ivug_crop_ug_start");
	IV_ASSERT(crop_ug != NULL);

	return true;
}

