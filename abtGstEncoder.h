#ifndef _ABT_GST_ENCODER_H
#define _ABT_GST_ENCODER_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include "abtMisc.h"

typedef struct {

	GstElement *pipeline;
	GstElement *src;
	GstElement *conv;
	GstElement *enc;
	GstElement *mux;
	GstElement *sink;

	GstCaps *caps_src;
	GstCaps *caps_src2conv;
	GstCaps *caps_conv2enc;
	
	GstBus *bus;
	guint bus_watch_id;

	GMainLoop *loop;

	GstBuffer *buffer;

} abtGstEncoderInfo;

int abtGstEncoderConstruct(abtGstEncoderInfo *, abtMediaInfo *);
int abtGstEncoderRun(abtGstEncoderInfo *, abtMediaInfo *);
int abtGstEncoderEnd(abtGstEncoderInfo *);
int abtGstEncoderDestroy(abtGstEncoderInfo *);

int _abtGstEncoderConstruct(abtGstEncoderInfo *, abtMediaInfo *);
int _abtGstEncoderRun(abtGstEncoderInfo *);
int _abtCountdown(void *);

static void cb_need_data(GstElement *, guint, gpointer);
static void link_to_multiplexer(GstPad *, GstElement *);

#endif
