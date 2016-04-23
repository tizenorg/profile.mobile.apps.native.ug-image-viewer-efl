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
#include "ivug-define.h"
#include "ivug-debug.h"
#include "ivug-transcoder.h"

#include "ivug-file-info.h"

#if 0
#include <gst/gst.h>

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-TRANS"


#define FRAME_RATE (2)

/*
gst-launch multifilesrc location="/opt/usr/media/DCIM/Camera/20130826-133700.jpg" caps="image/jpeg,framerate=2/1" loop=true do-timestamp=false num-buffers=5 ! jpegdec !
		videoflip method=1 ! videoscale ! capsfilter caps="video/x-raw-yuv,format=(fourcc)I420,width=720,height=1280,framerate=2/1" ! savsenc_mp4 gob-size=10 auto-setting=false data-partitioning=false ! mp4mux movie-timescale=1024 name=mux ! filesink location=rr1.mp4 sync=false async=false
filesrc location="/tmp/.ivug_t.wav" ! wavparse ! audioconvert ! savsenc_aac ! mux.

gst-launch filesrc location="/opt/usr/media/DCIM/Camera/20130819-124914.jpg" typefind=true ! jpegdec ! "video/x-raw-yuv,format=(fourcc)I420,framerate=1/1"  ! videoflip method=1 ! videoscale
! capsfilter caps="video/x-raw-yuv,format=(fourcc)I420,width=1280,height=720,framerate=1/1" ! savsenc_mp4 gop-size=1 ! mp4mux name=mux ! filesink location=rr1.mp4 sync=false
filesrc location="/tmp/.ivug_t.wav" ! wavparse ! audioconvert ! savsenc_aac ! mux.


*/

typedef struct _CustomData {
	GstElement *pipeline;
	GstElement *audioconvert;
} CustomData;

static void pad_added_handler(GstElement *src, GstPad *new_pad /* Created somtimes pad */, CustomData *data)
{
	GstPad *sink_pad = gst_element_get_static_pad(data->audioconvert, "sink");

	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;

	MSG_ERROR("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

	/* If our converter is already linked, we have nothing to do here */
	if (gst_pad_is_linked(sink_pad)) {
		MSG_ERROR("  We are already linked. Ignoring.");
		goto exit;
	}

	/* Check the new pad's type */
	new_pad_caps = gst_pad_get_caps(new_pad);
	new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
	new_pad_type = gst_structure_get_name(new_pad_struct);

	if (!g_str_has_prefix(new_pad_type, "audio/x-raw")) {
		MSG_ERROR("  It has type '%s' which is not raw audio. Ignoring.", new_pad_type);
		goto exit;
	}

	/* Attempt the link */
	ret = gst_pad_link(new_pad, sink_pad);
	if (GST_PAD_LINK_FAILED(ret)) {
		MSG_ERROR("  Type is '%s' but link failed.", new_pad_type);
	} else {
		MSG_ERROR("  Link succeeded (type '%s').", new_pad_type);
	}

exit:  /* Unreference the new pad's caps, if we got them */

	if (new_pad_caps != NULL) {
		gst_caps_unref(new_pad_caps);
	}

	/* Unreference the sink pad */
	gst_object_unref(sink_pad);
}



extern "C" bool ivug_trancoder_convert_video(const char *jpg, const char *wav, const char *outfile, IVTransConfig *config)
{
	MSG_ERROR("Jpeg=%s Wav=%s Out=%s Size=%d", jpg, wav, outfile, config->inSizeLimit);

	CustomData data;

	GstElement *pipeline;

	gst_init(NULL, NULL);

	pipeline = gst_pipeline_new("my-transcoder");

	/* create elements */
	GstElement *source = gst_element_factory_make("multifilesrc", "isource");		// Image source

	g_object_set(source, "location", jpg,
	             "caps", gst_caps_new_simple("image/jpeg",
	                     "framerate", GST_TYPE_FRACTION, FRAME_RATE, 1,
	                     NULL),
	             "do-timestamp", false,
	             "num-buffers", 5,
	             "loop", true,
	             NULL);

// Video path
	GstElement *decoder = gst_element_factory_make("jpegdec", "idecoder"); 	// Image decoder

	/*
		Orientation

		   1 : top left
		   2 : top right
		   3 : bottom right
		   4 : bottom left
		   5 : left top
		   6 : right top
		   7 : right bottom
		   8 : left bottom


		method				: method
							  flags: readable, writable, controllable
							  Enum "GstVideoFlipMethod" Default: 0, "none"
								 (0): none			   - Identity (no rotation)
								 (1): clockwise 	   - Rotate clockwise 90 degrees
								 (2): rotate-180	   - Rotate 180 degrees
								 (3): counterclockwise - Rotate counter-clockwise 90 degrees
								 (4): horizontal-flip  - Flip horizontally
								 (5): vertical-flip    - Flip vertically
								 (6): upper-left-diagonal - Flip across upper left/lower right diagonal
								 (7): upper-right-diagonal - Flip across upper right/lower left diagonal
	*/
	GstElement *vflip = gst_element_factory_make("videoflip", "vflip");

	int method[] = { 0, 0, 4, 2, 5, 0, 1, 0, 3 };

	g_object_set(vflip, "method", method[config->inOrientation], NULL);

	GstElement *vscale = gst_element_factory_make("videoscale", "vscale");

	GstElement *capsfilter = gst_element_factory_make("capsfilter", "cfilter");

	g_object_set(capsfilter, 	"caps",
	             gst_caps_new_simple("video/x-raw-yuv",
	                                 "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('I', '4', '2', '0'),
	                                 "width", G_TYPE_INT, config->inWidth,
	                                 "height", G_TYPE_INT, config->inHeight,
	                                 "framerate", GST_TYPE_FRACTION, FRAME_RATE, 1,
	                                 NULL),
	             NULL);
//	"video/x-raw-yuv,format=(fourcc)I420,width=800,height=400,framerate=1/1"


	GstElement *encoder = gst_element_factory_make("savsenc_mp4", "vencoder"); 	// Video Encoder

	g_object_set(encoder, 	"gob-size", 10,
	             "auto-setting", false,
	             "data-partitioning", false,
//			"hw-accel", false,
	             NULL);

// Audio path
	GstElement *asource = gst_element_factory_make("filesrc", "asource");		// Audio source
	MSG_ASSERT(asource != NULL);

	g_object_set(asource, "location", wav
//		, "do-timestamp", true
	             , NULL);

	GstElement *wavparser = gst_element_factory_make("wavparse", "wavparser");		// Wave parser
	MSG_ASSERT(wavparser != NULL);

	GstElement *audioconvert = gst_element_factory_make("audioconvert", "aconv");		// Wave parser
	MSG_ASSERT(audioconvert != NULL);

	/* Connect to the pad-added signal */
	data.audioconvert = audioconvert;
	g_signal_connect(wavparser, "pad-added", G_CALLBACK(pad_added_handler), &data);

	GstElement *aacenc = gst_element_factory_make("savsenc_aac", "aacenc");		// AAC Encoder
	MSG_ASSERT(aacenc != NULL);

// Multiplexed
	GstElement *muxer = gst_element_factory_make("mp4mux", "muxer");		// Muxer

	GstElement *sink = gst_element_factory_make("filesink", "fsink");		// file sink

	g_object_set(sink, "location", outfile,
	             "sync", false,
	             "async", false,
	             NULL);

	if (!source || !decoder || !vflip || !vscale || !capsfilter || !encoder ||
	        !asource || !wavparser || !audioconvert || !aacenc || !muxer || !sink) {
		MSG_ERROR("Some Element is not crated");
		return -1;
	}

	/* must add elements to pipeline before linking them */
	gst_bin_add_many(GST_BIN(pipeline),
	                 source, decoder, vflip, vscale, capsfilter, encoder,
	                 asource, wavparser, audioconvert, aacenc,
	                 muxer, sink,
	                 NULL);

	/* link */
	if (gst_element_link_many(source, decoder, vflip, vscale, capsfilter, encoder, NULL) != TRUE) {
		MSG_ERROR("Cannot link element 1");
		gst_object_unref(pipeline);
		return false;
	}

	if (gst_element_link_many(asource, wavparser, NULL) != TRUE) {
		MSG_ERROR("Cannot link element 2");
		gst_object_unref(pipeline);
		return false;
	}

	if (gst_element_link_many(audioconvert, aacenc, NULL) != TRUE) {
		MSG_ERROR("Cannot link element 3");
		gst_object_unref(pipeline);
		return false;
	}

	if (gst_element_link_many(muxer, sink, NULL) != TRUE) {
		MSG_ERROR("Cannot link element 4");
		gst_object_unref(pipeline);
		return false;
	}


	/*	Get request pad */
	GstPadTemplate *mux_asink_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(muxer), "audio_%d");
	GstPadTemplate *mux_vsink_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(muxer), "video_%d");

	GstPad *vpad, *apad;

	vpad = gst_element_request_pad(muxer, mux_vsink_pad_template, NULL, NULL);
	apad = gst_element_request_pad(muxer, mux_asink_pad_template, NULL, NULL);


	GstPad *enc_v_pad;
	enc_v_pad = gst_element_get_static_pad(encoder, "src");

	GstPad *enc_a_pad;
	enc_a_pad = gst_element_get_static_pad(aacenc, "src");

	if (gst_pad_link(enc_v_pad, vpad) != GST_PAD_LINK_OK ||
	        gst_pad_link(enc_a_pad, apad) != GST_PAD_LINK_OK) {
		MSG_ERROR("Tee could not be linked.");
		gst_object_unref(pipeline);
		return -1;
	}

	gst_object_unref(enc_v_pad);
	gst_object_unref(enc_a_pad);

	/* Start playing */
	GstStateChangeReturn ret;

	ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

	if (ret == GST_STATE_CHANGE_FAILURE) {
		MSG_ERROR("Unable to set the pipeline to the playing state.");
		gst_object_unref(pipeline);
		return false;
	}


	/* Listen to the bus */
	GstBus *bus;
	GstMessage *msg;
	bool terminate = false;

	bus = gst_element_get_bus(pipeline);

	do {
		msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,	(GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
		/* Parse message */
		if (msg != NULL) {
			GError *err;
			gchar *debug_info;

			switch (GST_MESSAGE_TYPE(msg)) {
			case GST_MESSAGE_ERROR:
				gst_message_parse_error(msg, &err, &debug_info);
				MSG_ERROR("Error received from element %s: %s", GST_OBJECT_NAME(msg->src), err->message);
				MSG_ERROR("Debugging information: %s", debug_info ? debug_info : "none");
				g_clear_error(&err);
				g_free(debug_info);
				terminate = TRUE;
				break;

			case GST_MESSAGE_EOS:
				MSG_ERROR("End-Of-Stream reached.");
				terminate = TRUE;
				break;
			case GST_MESSAGE_STATE_CHANGED:
				/* We are only interested in state-changed messages from the pipeline */
				if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline)) {
					GstState old_state, new_state, pending_state;
					gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
					MSG_ERROR("Pipeline state changed from %s to %s:",	gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
				}
				break;
			default:		  /* We should not reach here */
				MSG_ERROR("Unexpected message received.");
				break;
			}

			gst_message_unref(msg);
		}
	} while (!terminate);

	/*
	gst_element_release_request_pad (tee, tee_audio_pad);  gst_element_release_request_pad (tee, tee_video_pad);  gst_object_unref (tee_audio_pad);  gst_object_unref (tee_video_pad);

	*/
	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	return true;
}


#else
extern "C" bool ivug_trancoder_convert_video(const char *jpg, const char *wav, const char *outfile, IVTransConfig *config)
{
	//MSG_ERROR("Feature is disabled");
	return false;
}
#endif
