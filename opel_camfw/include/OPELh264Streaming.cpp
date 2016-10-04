#include "OPELh264Streaming.h"
OPELH264Streaming *OPELH264Streaming::opel_h264_streaming = NULL;

static void setH264Parse(typeElement *element)
{
	assert(element != NULL);
	const char* n_h264parse = "h264parse";
	std::string *str = new std::string(n_h264parse);
	element->name = str;
	element->type = kNO_PROP;
}

static void setgdpPay(typeElement *element)
{
	assert(element != NULL);
	const char* n_gdppay = "gdppay";
	std::string *str = new std::string(n_gdppay);
	element->name = str;
	element->type = kNO_PROP;
}
static void setRtph264Pay(typeElement *element)
{
	assert(element != NULL);
	const char* n_rtph264pay = "rtph264pay";
	std::string *str = new std::string(n_rtph264pay);
	element->name = str;
	element->type = kNO_PROP;
}
static bool h264StreamingPropSetting(std::vector<typeElement*>* _v_type_element,
		dbusStreamingRequest* _stream_request)
{
	assert(_v_type_element != NULL && _stream_request != NULL);
	typeElement *_conv = findByElementName(_v_type_element, 
			"nvvidconv");
	typeElement *_enc = findByElementName(_v_type_element, 
			"omxh264enc");
	typeElement *_udp_sink = findByElementName(_v_type_element, 
			"tcpserversink");
	typeElement *_rtph_264pay = findByElementName(_v_type_element, 
			"rtph264pay");

	if(!_conv || !_enc || !_udp_sink || !_rtph_264pay)
	{
		OPEL_DBG_ERR("Elements are NULL");
		return false;
	}
	g_object_set(G_OBJECT(_enc->element), "control-rate", (guint)2, 
			"bitrate", (guint)2000000, NULL); 
	g_object_set(G_OBJECT(_rtph_264pay->element), "pt", (guint)96, 
			"config-interval", (guint)1, NULL);
	g_object_set(G_OBJECT(_udp_sink->element), "host", 
			_stream_request->ip_address, "port",
			_stream_request->port, "sync",
			FALSE, NULL);

	return true;
}
OPELH264Streaming *OPELH264Streaming::getInstance(void)
{
	if(opel_h264_streaming == NULL)
		opel_h264_streaming = new OPELH264Streaming();
	return opel_h264_streaming;
}

OPELH264Streaming::OPELH264Streaming()
{
	this->_v_fly_type_element = 
		new std::vector<typeElement*>(OPEL_NUM_DEFAULT_STREAMING_ELE);
	this->is_streaming_run = false;
}

OPELH264Streaming::~OPELH264Streaming()
{
   	
}

bool OPELH264Streaming::defaultStreamingFactory(void)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;

	typeElement *_queue = findByElementName(this->_v_type_element,
			"queue");
	typeElement *_conv = findByElementNameNSubType(this->_v_type_element, 
			"nvvidconv", kBGR);
	typeElement *_enc = findByElementName(this->_v_type_element, 
			"omxh264enc");
	typeElement *_udp_sink = findByElementName(this->_v_type_element,
				"tcpserversink");

	//need to copy
	typeElement *_new_queue = (typeElement*)malloc(sizeof(typeElement)); 
	_new_queue->element_prop = _queue->element_prop;
	initializeTypeElement(_new_queue, _new_queue->element_prop);
	
	typeElement *_new_conv = (typeElement*)malloc(sizeof(typeElement));
	_new_conv->element_prop = _conv->element_prop;
	initializeTypeElement(_new_conv, _new_conv->element_prop);

	typeElement *_new_enc  = (typeElement*)malloc(sizeof(typeElement));
	_new_enc->element_prop = _enc->element_prop;
	initializeTypeElement(_new_enc, _new_enc->element_prop);

	typeElement *_new_udp_sink = (typeElement*)malloc(sizeof(typeElement));
	_new_udp_sink->element_prop = _udp_sink->element_prop;
	initializeTypeElement(_new_udp_sink, _new_udp_sink->element_prop);
	
	typeElement *_h264_parse = (typeElement*)malloc(sizeof(typeElement));
	setH264Parse(_h264_parse);	

	typeElement *_rtph_264_pay = (typeElement*)malloc(sizeof(typeElement));
	setRtph264Pay(_rtph_264_pay);
	
	typeElement *_gdppay = (typeElement*)malloc(sizeof(typeElement));
	setgdpPay(_gdppay);

  (*this->_v_fly_type_element)[0] = _new_queue;
  (*this->_v_fly_type_element)[1] = _new_conv;
  (*this->_v_fly_type_element)[2] = _new_enc;
  (*this->_v_fly_type_element)[3] = _h264_parse;
  (*this->_v_fly_type_element)[4] = _rtph_264_pay;
  (*this->_v_fly_type_element)[5] = _gdppay;
  (*this->_v_fly_type_element)[6] = _new_udp_sink;

	gstElementFactory(this->_v_fly_type_element);
	
	h264StreamingPropSetting(this->_v_fly_type_element, this->streaming_handle);

	__OPEL_FUNCTION_EXIT__;
	return true;
}

bool OPELH264Streaming::defaultStreamingPipelineAdd(GstElement *pipeline)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL
			&& pipeline != NULL);
	__OPEL_FUNCTION_ENTER__;
	bool ret = true;

	typeElement *_queue = findByElementName(this->_v_fly_type_element,
			"queue");
	typeElement *_conv = findByElementNameNSubType(this->_v_fly_type_element, 
			"nvvidconv", kBGR);
	typeElement *_enc = findByElementName(this->_v_fly_type_element, 
			"omxh264enc");
	typeElement *_parse = findByElementName(this->_v_fly_type_element,
				"h264parse");
	typeElement *_pay = findByElementName(this->_v_fly_type_element,
				"rtph264pay");
	typeElement *_gdppay = findByElementName(this->_v_fly_type_element,
			"gdppay");
	typeElement *_udp_sink = findByElementName(this->_v_fly_type_element,
			"tcpserversink");
	if(!_queue || !_conv || !_enc || !_parse || !_pay || !_udp_sink || !_gdppay)
	{
		OPEL_DBG_ERR("elements are NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}

	gst_bin_add_many(GST_BIN(pipeline), _queue->element, _conv->element,
			_enc->element, _parse->element, _pay->element, _gdppay->element, 
			_udp_sink->element, NULL);

	ret = gst_element_link(_queue->element, _conv->element);
	if(!ret)
	{
		OPEL_DBG_ERR("element link failed");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	ret = gst_element_link_filtered(_conv->element, _enc->element, _conv->caps);
	if(!ret)
	{
		OPEL_DBG_ERR("element link filtered failed");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	ret = gst_element_link_filtered(_enc->element, _parse->element, _enc->caps);
	if(!ret)
	{
		OPEL_DBG_ERR("element link filtered failed");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	ret = gst_element_link_many(_parse->element, _pay->element, 
		 _gdppay->element, _udp_sink->element, NULL);
	if(!ret)
	{
		OPEL_DBG_ERR("element link many failed");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}

	__OPEL_FUNCTION_EXIT__;
	return true; 
}

bool OPELH264Streaming::defaultStreamingCapFactory(void)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	char caps_buffer[256];
	char caps_buffer_enc[256];
	typeElement *_conv = findByElementNameNSubType(this->_v_fly_type_element, 
			"nvvidconv", kBGR);
	typeElement *_enc = findByElementName(this->_v_fly_type_element, 
			"omxh264enc");
  
	if(!_conv || !_enc)
	{
		OPEL_DBG_ERR("Elements are NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}

	gint width = (guint)STREAMING_720P_WIDTH;
	gint height = (guint)STREAMING_720P_HEIGHT;

 	sprintf(caps_buffer, "video/x-raw(memory:NVMM), width=(int){%d},\
			height=(int){%d}", width, height);
	
	_conv->caps = gst_caps_from_string(caps_buffer);
	
	sprintf(caps_buffer_enc, "video/x-h264, stream-format=(string)byte-stream");

	_enc->caps = gst_caps_from_string(caps_buffer_enc);

	if(!_conv->caps || !_enc->caps)
	{
		OPEL_DBG_ERR("Caps are NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	
	__OPEL_FUNCTION_EXIT__;
	return true;
}

void OPELH264Streaming::defaultStreamingPadLink(GstPad *tee_src_pad)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL
			&& tee_src_pad != NULL);
	__OPEL_FUNCTION_ENTER__;
	typeElement *_queue = findByElementName(this->_v_fly_type_element,
			"queue");
	if(!_queue)
	{
		OPEL_DBG_ERR("queue is NULL");
		__OPEL_FUNCTION_ENTER__;
		return;
	}
	_queue->pad = gst_element_get_static_pad(_queue->element, "sink");
	gst_pad_link(tee_src_pad, _queue->pad);
	__OPEL_FUNCTION_EXIT__;
}


void OPELH264Streaming::defaultStreamingSyncStateWithParent(void)
{
	assert(this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	OPELgstSyncStateWithParent(this->_v_fly_type_element);
	__OPEL_FUNCTION_EXIT__;
}


bool streamingStart(std::vector<typeElement*> *_type_element_v, 
		OPELH264Streaming *request_handle)
{
	assert(_type_element_v != NULL && request_handle != NULL);
	__OPEL_FUNCTION_ENTER__;

	bool ret = true;
	GstPadTemplate *templ;
	std::string tmp_str;
	std::vector<typeElement*> *_fly_type_element_v;
	GstPad *src_pad;

	request_handle->setTypeElementVector(_type_element_v);

	OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();

	GstElement *pipeline = tx1->getPipeline();


	request_handle->defaultStreamingFactory();
	_fly_type_element_v = request_handle->getFlyTypeElementVector();

	typeElement *tee = findByElementName(_type_element_v, "tee");
	typeElement *queue = findByElementName(_fly_type_element_v, "queue");

	if(!tee || !queue)
	{
		OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
		__OPEL_FUNCTION_EXIT__;
		return NULL;
	}

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee->element), "src_%u");
	src_pad = gst_element_request_pad(tee->element, templ, NULL, NULL);

	request_handle->setSrcPad(src_pad);

	//#if OPEL_LOG_VERBOSE
	OPEL_DBG_VERB("Obtained request pad %s for %s", gst_pad_get_name(templ),
			tee->name->c_str());
	//#endif

	request_handle->defaultStreamingCapFactory();
	request_handle->defaultStreamingPipelineAdd(pipeline);
	request_handle->defaultStreamingPadLink(request_handle->getSrcPad());

	if(!(tx1->getIsPlaying()))
	{
		ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_READY);
		ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
		tx1->setIsPlaying(true);
	}
	else
		request_handle->defaultStreamingSyncStateWithParent();
	
	__OPEL_FUNCTION_EXIT__;
	return request_handle;
}

static GstPadProbeReturn detachStreamingCB(GstPad *pad, GstPadProbeInfo *info,
		gpointer user_data)
{
	assert(user_data != NULL);
	__OPEL_FUNCTION_ENTER__;

	OPELH264Streaming *request_handle = NULL;
	OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
	request_handle = (OPELH264Streaming*)user_data;

	std::vector<typeElement*> *_v_type_element = 
		request_handle->getTypeElementVector();
	
	std::vector<typeElement*> *v_fly_type_elements =
		request_handle->getFlyTypeElementVector();

	typeElement *_tee = findByElementName(_v_type_element,
			"tee");
	if(!_tee)
	{
		OPEL_DBG_ERR("tee is NULL");
		return GST_PAD_PROBE_OK;
	}
	for(int i=0; i<OPEL_NUM_DEFAULT_STREAMING_ELE; i++)
	{
		std::cout << "Removed Elements : " << (*v_fly_type_elements)[i]->name->c_str()
			<< std::endl;
		gst_element_set_state((*v_fly_type_elements)[i]->element, GST_STATE_NULL);
		gst_bin_remove(GST_BIN(tx1->getPipeline()), (*v_fly_type_elements)[i]->element);
		delete (*v_fly_type_elements)[i];
	}
	gst_element_release_request_pad(_tee->element,
			request_handle->getSrcPad());
	
	request_handle->setIsStreamingRun(false);

	__OPEL_FUNCTION_EXIT__;
	return GST_PAD_PROBE_OK;
}

bool OPELH264Streaming::detachedStreaming(void)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	GstPad *sink_pad;	
	typeElement *_queue = findByElementName(this->_v_fly_type_element,
			            "queue");

	if(!_queue || !this->getSrcPad())
	{
		OPEL_DBG_ERR("queue and tee are NULL");
		return false;
	}
	gst_pad_add_probe(this->getSrcPad(), GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
			detachStreamingCB, (gpointer)this, NULL);

	sink_pad = gst_element_get_static_pad(_queue->element, "sink");
	gst_pad_unlink(this->getSrcPad(), sink_pad);

	checkRemainRequest();

	gst_pad_send_event(sink_pad, gst_event_new_eos());
	gst_object_unref(sink_pad);

	__OPEL_FUNCTION_EXIT__;
	return true;
}
