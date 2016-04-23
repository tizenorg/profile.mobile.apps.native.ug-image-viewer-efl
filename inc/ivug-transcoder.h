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

#ifndef __IFUG_TRANSCODER_H__
#define __IFUG_TRANSCODER_H__

/*
gst-launch multifilesrc location="341410.jpg" caps="image/jpeg,framerate=1/1" num-buffers=10 ! jpegdec ! savsenc_mp4 ! mp4mux name=mux ! filesink location=kk.mp4 sync=false
            filesrc location="deltio.mp3" ! mp3parse ! mux.
*/

typedef struct {
	int inWidth;
	int inHeight;

	int inOrientation;
	int inSizeLimit;

	int outOutsize;

} IVTransConfig;

#ifdef __cplusplus
extern "C" {
#endif

bool ivug_trancoder_convert_video(const char *jpg, const char *wav, const char *outfile, IVTransConfig *config);


#ifdef __cplusplus
}
#endif


#endif 		// __IFUG_TRANSCODER_H__


