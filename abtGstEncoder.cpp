#include "abtGstEncoder.h"


int abtGstEncoderConstruct(abtGstEncoderInfo *pGstEncInfo, abtMediaInfo *pMedInfo) {

	// init Gstreamer
	// gst_init(&argc, &argv)
	gst_init(NULL, NULL);

	_abtGstEncoderConstruct(pGstEncInfo, pMedInfo);

	// setup pipeline
	gst_bin_add_many(GST_BIN(pGstEncInfo->pipeline),
			pGstEncInfo->src, pGstEncInfo->conv, pGstEncInfo->enc, pGstEncInfo->mux, pGstEncInfo->sink, NULL);
	gst_element_link_many(pGstEncInfo->src, pGstEncInfo->conv, pGstEncInfo->enc, NULL);
	gst_element_link(pGstEncInfo->mux, pGstEncInfo->sink);

	gst_element_link_filtered(pGstEncInfo->src, pGstEncInfo->conv, pGstEncInfo->caps_src2conv);
	gst_element_link_filtered(pGstEncInfo->conv, pGstEncInfo->enc, pGstEncInfo->caps_conv2enc);

	link_to_multiplexer(gst_element_get_static_pad(pGstEncInfo->enc, "src"), pGstEncInfo->mux);

	return 0;

}


// --------------------------------------------------------------- //
// --------------------------- Warning --------------------------- //
// -------------------- Asynchronous Function -------------------- //
// --------------------------------------------------------------- //
int abtGstEncoderRun(abtGstEncoderInfo *pGstEncInfo, abtMediaInfo *pMedInfo) {

	pthread_t tid1;
	pthread_t tid2;

	static uint64_t data[2] = {0x00000000, 0x00000000};

	data[0] = (uint64_t) pGstEncInfo;
	data[1] = (uint64_t) pMedInfo;

	pthread_create(&tid1,
			NULL,
			(void * (*)(void *)) _abtCountdown,
			(void *) data);

	pthread_create(&tid2,
			NULL,
			(void * (*)(void *)) _abtGstEncoderRun,
			(void *) pGstEncInfo);

	return 0;

}
// --------------------------------------------------------------- //
// --------------------------------------------------------------- //


int abtGstEncoderEnd(abtGstEncoderInfo *pGstEncInfo) {

	gst_element_set_state(pGstEncInfo->pipeline, GST_STATE_NULL);

	return 0;

}


int abtGstEncoderDestroy(abtGstEncoderInfo *pGstEncInfo) {

	gst_object_unref(GST_OBJECT(pGstEncInfo->pipeline));
	g_source_remove(pGstEncInfo->bus_watch_id);
	printf("i\n");
	g_main_loop_unref(pGstEncInfo->loop);
	printf("ii\n");
	gst_object_unref(pGstEncInfo->bus);
	printf("iii\n");
	//gst_caps_unref(pGstEncInfo->caps_src);
	printf("iiii\n");
	gst_caps_unref(pGstEncInfo->caps_src2conv);
	printf("iiiii\n");
	gst_caps_unref(pGstEncInfo->caps_conv2enc);

	return 0;

}


int _abtGstEncoderConstruct(abtGstEncoderInfo *pGstEncInfo, abtMediaInfo *pMedInfo) {

	char *s = NULL;
	static uint64_t data[2] = {0x00000000, 0x00000000};

	s = (char *) calloc(ABT_FILE_PATH_LENGTH + ABT_FILE_NAME_LENGTH, sizeof(char));


	pGstEncInfo->loop = g_main_loop_new(NULL, FALSE);

	pGstEncInfo->pipeline = gst_pipeline_new("pipeline");
	pGstEncInfo->src = gst_element_factory_make("appsrc", NULL);
	pGstEncInfo->conv = gst_element_factory_make("videoconvert", NULL);
	pGstEncInfo->enc = gst_element_factory_make("omxh264enc", NULL);
	pGstEncInfo->mux = gst_element_factory_make("mp4mux", NULL);
	pGstEncInfo->sink = gst_element_factory_make("filesink", NULL);

	pGstEncInfo->buffer = gst_buffer_new_allocate(NULL,
			(pMedInfo->width)*(pMedInfo->height)*(pMedInfo->channel),
			NULL);

	if (pGstEncInfo->buffer == NULL) { g_print("buffer error\n"); exit(0); }

	if (pGstEncInfo->src == NULL) { g_print("src error\n"); return 0; }
	if (pGstEncInfo->conv == NULL) {	g_print("conv error\n"); return 0; }
	if (pGstEncInfo->enc == NULL) { g_print("enc error\n"); return 0; }
	if (pGstEncInfo->mux == NULL) { g_print("mux error\n"); return 0; }
	if (pGstEncInfo->sink == NULL) { g_print("sink error\n"); return 0; }

	pGstEncInfo->caps_src2conv = gst_caps_new_simple("video/x-raw",
			"width", G_TYPE_INT, pMedInfo->width,
			"height", G_TYPE_INT, pMedInfo->height,
			"framerate", GST_TYPE_FRACTION, pMedInfo->fps, 1,
			"format", G_TYPE_STRING, "YUY2", NULL);

	pGstEncInfo->caps_conv2enc = gst_caps_new_simple("video/x-raw",
			"width", G_TYPE_INT, pMedInfo->width,
			"height", G_TYPE_INT, pMedInfo->height,
			"framerate", GST_TYPE_FRACTION, pMedInfo->fps, 1,
			"format", G_TYPE_STRING, "NV12", NULL);

	g_object_set(G_OBJECT(pGstEncInfo->src), "caps", 
		 gst_caps_new_simple("video/x-raw", 
		 "width", G_TYPE_INT, pMedInfo->width, 
		 "height", G_TYPE_INT, pMedInfo->height, 
		 "framerate", GST_TYPE_FRACTION, pMedInfo->fps, 1, 
		 "format", G_TYPE_STRING, "YUY2", 
		 NULL), NULL);

	g_object_set(G_OBJECT(pGstEncInfo->src), 
			"stream-type", 0,
			"format", GST_FORMAT_TIME, NULL);

	data[0] = ((uint64_t) pGstEncInfo);
	data[1] = ((uint64_t) pMedInfo);
	g_signal_connect(pGstEncInfo->src, "need-data", G_CALLBACK(cb_need_data), (void *) data);

	sprintf(s, "%s%s%s", pMedInfo->file_path, pMedInfo->file_name, pMedInfo->file_ext);
	g_object_set(G_OBJECT(pGstEncInfo->sink), "location", s, NULL);

	pGstEncInfo->bus = gst_pipeline_get_bus(GST_PIPELINE(pGstEncInfo->pipeline));
	//gst_bus_add_watch(pGstEncInfo->bus, bus_call, pGstEncInfo->loop);

	return 0;

}


int _abtGstEncoderRun(abtGstEncoderInfo *pGstEncInfo) {

	gst_element_set_state(pGstEncInfo->pipeline, GST_STATE_PLAYING);
	g_main_loop_run(pGstEncInfo->loop); 

	return 0;

}

int _abtCountdown(void *data) {

	abtGstEncoderInfo *pGstEncInfo = (abtGstEncoderInfo *)(((uint64_t *) data)[0]);
	abtMediaInfo *pMedInfo = (abtMediaInfo *)(((uint64_t *) data)[1]);

	sleep(pMedInfo->time_length);
	gst_app_src_end_of_stream(GST_APP_SRC(pGstEncInfo->src));

	return 0;

}

static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data) {

	abtGstEncoderInfo *pGstEncInfo = (abtGstEncoderInfo *)(((uint64_t *) user_data)[0]);
	abtMediaInfo *pMedInfo = (abtMediaInfo *)(((uint64_t *) user_data)[1]);

	static gboolean white = FALSE;
	static GstClockTime timestamp = 0;
	static int frame_number = 0;

	GstFlowReturn ret;



	int i = 0;

	// jQueryFrame(fd, &jImg);
	abtQueryFrame(pMedInfo);

	gsize size = 0;

	size = gst_buffer_fill(pGstEncInfo->buffer,
			0, 
			(pMedInfo->data),
			(pMedInfo->width)*(pMedInfo->height)*(pMedInfo->channel) );

	//if (size != 800*450*2) { printf("buffer error"); exit(0); }

	/* this makes the image black/white */
	//gst_buffer_memset(buffer, 0, white ? 0xff : 0x00, 640*480*4);
	//buffer.offset = frame_number;
	//
	//
	//for (i = 0; i < 640*480*2; i++) {
		//gst_buffer_memset(buffer, i, jImg.img[i], 1);
		//buffer[i] = jImg.img[i];
	//}

	//white = (!white);

	GST_BUFFER_PTS(pGstEncInfo->buffer) = timestamp;
	GST_BUFFER_DURATION(pGstEncInfo->buffer) = gst_util_uint64_scale_int(1, GST_SECOND, pMedInfo->fps);
	GST_BUFFER_OFFSET(pGstEncInfo->buffer) = frame_number;
	frame_number++;

	timestamp += GST_BUFFER_DURATION(pGstEncInfo->buffer);

	g_signal_emit_by_name(appsrc, "push-buffer", pGstEncInfo->buffer, &ret);
	//g_signal_emit_by_name(appsrc, "push-buffer", buf, &ret);
	g_print("callback end nearly\n");

	if (ret != GST_FLOW_OK) {
		/* something wrong, stop pushing */
		g_print("Error on ret\n");
		//g_main_loop_quit(loop);
	}
}

static void link_to_multiplexer(GstPad *tolink_pad, GstElement *mux)
{
	GstPad *pad;
	gchar *srcname, *sinkname;
	GstCaps *caps;
	caps = gst_caps_new_simple("video/x-h264", 
			//"width", G_TYPE_INT, 1280,
			//"height", G_TYPE_INT, 720,
			//"framerate", GST_TYPE_FRACTION, 24, 1,
			"stream-format", G_TYPE_STRING, "avc",
			"alignment", G_TYPE_STRING, "au",
			NULL);
	srcname = gst_pad_get_name(tolink_pad);
	pad = gst_element_get_compatible_pad (mux, tolink_pad, caps);
	if (pad == NULL) { g_print("pad is NULL\n"); }
	//gst_pad_link (tolink_pad, pad);
	gst_pad_link (tolink_pad, pad);
	sinkname = gst_pad_get_name (pad);
	gst_object_unref (GST_OBJECT (pad));

	g_print ("A new pad %s was created and linked to %s\n", sinkname, srcname);
	g_free (sinkname);
	g_free (srcname);
}

