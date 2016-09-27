#include "OPELrawRequest.h"
#include "OPELgstElementTx1.h"

OPELRawRequest *OPELRawRequest::opel_raw_request = NULL;

OPELRawRequest *OPELRawRequest::getInstance(void)
{
	if(opel_raw_request == NULL)
		opel_raw_request = new OPELRawRequest();
	return opel_raw_request;
}

OPELRawRequest::OPELRawRequest()
{
	this->app_sink = (typeElement*)malloc(sizeof(typeElement));
	this->_v_fly_type_element = 
		new std::vector<typeElement*>(OPEL_NUM_DEFAULT_RAW_ELE);
}

OPELRawRequest::~OPELRawRequest()
{
	if(this->app_sink != NULL)
	  free(this->app_sink);
	
	if(this->_v_fly_type_element != NULL)
		delete this->_v_fly_type_element;
}

static void initAppSinkElement(typeElement* app_sink)
{
	assert(app_sink != NULL);
	const char* n_app_sink = "appsink";
	std::string *str = new std::string(n_app_sink);
	app_sink->name = str;
	app_sink->type = kAPP_SINK;
	
}
static void initAppSinkElementProp(typeElement* app_sink)
{
	assert(app_sink != NULL);
}

bool OPELRawRequest::defaultOpenCVElementFactory()
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	
	this->num_iter = OPEL_NUM_DEFAULT_RAW_ELE;

	typeElement *_conv = findByElementNameNSubType(this->_v_type_element,
			      "nvvidconv", kBGR);
	typeElement *_queue = findByElementName(this->_v_type_element,
			      "queue");
	if(!_conv || !this->app_sink || !_queue)
	{
		OPEL_DBG_ERR("conv and app_sink is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
 	
	typeElement *_new_conv = (typeElement*)malloc(sizeof(typeElement));
	_new_conv->element_prop = _conv->element_prop;
	initializeTypeElement(_new_conv, _new_conv->element_prop);

	typeElement *_new_queue = (typeElement*)malloc(sizeof(typeElement));
	_new_queue->element_prop = _queue->element_prop;
	initializeTypeElement(_new_queue, _new_queue->element_prop);
	
	initAppSinkElement(this->app_sink);	
	
	(*this->_v_fly_type_element)[0] = _new_queue;
	(*this->_v_fly_type_element)[1] = _new_conv;
	(*this->_v_fly_type_element)[2] = this->app_sink;
  
	gstElementFactory(this->_v_fly_type_element);

	_new_conv->prop->setGstObjectProperty(_new_conv->element);
	initAppSinkElementProp(this->app_sink);
	
	__OPEL_FUNCTION_EXIT__;
	return true;
}

bool OPELRawRequest::defaultOpenCVCapFactory()
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
 	__OPEL_FUNCTION_ENTER__;
	char caps_buffer[256];

	gint width = RAW_FORMAT_WIDTH;
	gint height = RAW_FORMAT_HEIGHT;

	typeElement *_conv = findByElementNameNSubType(this->_v_fly_type_element,
			      "nvvidconv", kBGR);
	
	if(!_conv)
	{
		OPEL_DBG_ERR("conv element is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	sprintf(caps_buffer, "video/x-raw(memory:NVMM), format=(string){BGRx},\
			width=(int){%d}, height=(int){%d}", width, height);
	
	_conv->caps = gst_caps_from_string(caps_buffer);	

	if(!_conv->caps)
	{
		OPEL_DBG_ERR("Conv Caps is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	
	__OPEL_FUNCTION_EXIT__;
	return true;
}

bool OPELRawRequest::defaultOpenCVElementPipelineAdd(GstElement *pipeline)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL
			&& pipeline != NULL);

 	__OPEL_FUNCTION_ENTER__;
	bool ret = true;
	typeElement *_queue = findByElementName(this->_v_fly_type_element,
			      "queue");
	typeElement *_conv = findByElementNameNSubType(this->_v_fly_type_element,
			      "nvvidconv", kBGR);
	typeElement *_app_sink = findByElementName(this->_v_fly_type_element,
			      "appsink");

	if(!_conv || !_app_sink || !_queue)
	{
		OPEL_DBG_ERR("conv app_sink is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}

	gst_bin_add_many(GST_BIN(pipeline), _queue->element, _conv->element, 
			_app_sink->element, NULL);
	
	ret = gst_element_link(_queue->element, _conv->element);
	if(!ret)
	{
		OPEL_DBG_ERR("Gst element link failed");
		__OPEL_FUNCTION_EXIT__; 
		return ret;
	}
	
	ret = gst_element_link_filtered(_conv->element, _app_sink->element, _conv->caps);
	if(!ret)
	{
		OPEL_DBG_ERR("Gst element link filtered failed");
		__OPEL_FUNCTION_EXIT__; 
		return ret;
	}
			
	
	__OPEL_FUNCTION_EXIT__;
	return ret;
}

void OPELRawRequest::defaultOpenCVPadLink(GstPad *main_tee_src_pad)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL
			&& main_tee_src_pad != NULL);
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
	gst_pad_link(main_tee_src_pad, _queue->pad);
	__OPEL_FUNCTION_EXIT__;
}

void OPELRawRequest::defaultOpenCVGstSyncStateWithParent(void)
{
	assert(this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	OPELgstSyncStateWithParent(this->_v_fly_type_element);
	__OPEL_FUNCTION_EXIT__;
}
