#include "OPELgstElementTx1.h"
#include "OPELcommonUtil.h"
#include "OPELglobalRequest.h"
#include "OPELh264Streaming.h"

//OPELGstElementTx1 *OPELGstElementTx1::opel_gst_element_tx1 = NULL; 
OPELGstElementTx1 *OPELGstElementTx1::opel_gst_element_tx1[2]; 


void printTypeElement(std::vector<typeElement*> *_type_element_vector)
{
  assert(_type_element_vector != NULL);

  for(int i=0; i<_type_element_vector->size(); i++)
  {
    std::cout << *((*_type_element_vector)[i]->name) << std::endl;
    std::cout << *((*_type_element_vector)[i]->nickname) << std::endl;
  }
}
void inline setTypeProperty(unsigned _sub_type,
    typeElement *_type_element, ElementProperty *element_property)
{
  assert(_type_element != NULL && element_property != NULL);
  switch(_sub_type)
  {
    case kCAM:
      _type_element->prop = element_property->camProp;
      break;
    case kRTSP:
      _type_element->prop = element_property->rtspProp;
      break;
    case kI420:
      _type_element->prop = element_property->conProp;
      break;
    case kBGR:
      _type_element->prop = element_property->conProp;
      break;
    case kH264:
      _type_element->prop = element_property->encProp;
      break;
    case kNVJPEG:
      _type_element->prop = NULL;
      break;
    case kMP4:
      _type_element->prop = NULL;
      break;
    case kREC_SINK:
      _type_element->prop = element_property->fileProp;
      break;
    case kJPEG_SINK:
      _type_element->prop = element_property->fileProp;
      break;
    case kUDP_SINK:
      _type_element->prop = element_property->udpProp;
      break;
    case kNO_PROP:
      _type_element->prop = NULL;
      break;
    case kAPP_SINK:
      _type_element->prop = element_property->appSrcProp;
      break;
    default:
      OPEL_DBG_ERR("Invalid Sub-Type");
      break;
  }
}

OPELEventRecRequest::OPELEventRecRequest() {

}

OPELEventRecRequest::~OPELEventRecRequest() {

}

bool OPELEventRecRequest::pipelineMake() {
	__OPEL_FUNCTION_ENTER__;

  GstElement *tcpsrc, *queue2, *gdpdepay, *rtph264depay, *h264parse, *mp4mux, *filesink;
  std::vector<GstElement*> *element_vector = new std::vector<GstElement*>;

	// Element Factory
	OPEL_GST_ELEMENT_FACTORY_MAKE(this->pipeline, "pipeline", NULL);
	OPEL_GST_ELEMENT_FACTORY_MAKE(tcpsrc, "tcpclientsrc", NULL);
	OPEL_GST_ELEMENT_FACTORY_MAKE(queue2, "queue2", NULL);
	OPEL_GST_ELEMENT_FACTORY_MAKE(gdpdepay, "gdpdepay", NULL);
	OPEL_GST_ELEMENT_FACTORY_MAKE(rtph264depay, "rtph264depay", NULL);
	OPEL_GST_ELEMENT_FACTORY_MAKE(h264parse, "h264parse", NULL);
	OPEL_GST_ELEMENT_FACTORY_MAKE(mp4mux, "mp4mux", NULL);
	OPEL_GST_ELEMENT_FACTORY_MAKE(filesink, "filesink", NULL);

	// Element Property
	g_object_set(G_OBJECT(tcpsrc), "host", "127.0.0.1",
			"port", this->port, NULL);
	g_object_set(G_OBJECT(filesink), "location", this->file_path, NULL);

  element_vector->push_back(tcpsrc);
  element_vector->push_back(queue2);
  element_vector->push_back(gdpdepay);
  element_vector->push_back(rtph264depay);
  element_vector->push_back(h264parse);
  element_vector->push_back(mp4mux);
  element_vector->push_back(filesink);

  this->_element_vector = element_vector;


	// Pipeline Add
	gst_bin_add_many(GST_BIN(this->pipeline), tcpsrc, queue2, gdpdepay, rtph264depay,
			h264parse, mp4mux, filesink, NULL);
	gst_element_link_many(tcpsrc, queue2, gdpdepay, rtph264depay, h264parse, mp4mux,
      filesink, NULL);
	
	__OPEL_FUNCTION_EXIT__;
	return true;
}

void OPELGstElementTx1::InitInstance(void) {
  OPELGstElementTx1 *tmp;
  tmp = opel_gst_element_tx1[0] = new OPELGstElementTx1();
  tmp->setCameraNum(0);
  tmp = opel_gst_element_tx1[1] = new OPELGstElementTx1();
  tmp->setCameraNum(1);

}

OPELH264Streaming* OPELGstElementTx1::getStreamingRequest(void) {
	if (this->streaming_request == NULL)
		this->streaming_request = new OPELH264Streaming();
	return this->streaming_request;
}

OPELH264Streaming* OPELGstElementTx1::getDelayRequest(void) {
	if (this->delay_request == NULL)
		this->delay_request = new OPELH264Streaming();
	return this->delay_request;
}

void initializeTypeElement(typeElement *type_element, 
    ElementProperty *element_property)
{
  assert(type_element != NULL && element_property != NULL);

  type_element->name = new std::string(element_property->getElementName());
  type_element->nickname = new std::string(element_property->getElementNickName());
  type_element->element_prop = element_property;
  type_element->type = element_property->getElementType();
  type_element->sub_type = element_property->getSubType();
  setTypeProperty(type_element->sub_type, type_element, element_property); 
}

void OPELGstElementTx1::setElementPropertyVector(std::vector<ElementProperty*>
    *__v_element_property)
{
  assert(__v_element_property != NULL);

  this->_v_element_property = __v_element_property;
  for(int i=0; i< this->_v_element_property->size(); i++)
  {
    typeElement *tmp = (typeElement*)malloc(sizeof(typeElement));
    initializeTypeElement(tmp, (*this->_v_element_property)[i]);
    this->_type_element_vector->push_back(tmp);
    tmp = NULL;
  }

  // XXX
  if (this->camera_num == 1) {
    typeElement *tmp = (typeElement*)malloc(sizeof(typeElement));
    ElementProperty *element_prop = new ElementProperty(kSRC, kCAM, "v4l2src", "camerasrc2");
    //tmp->name = new std::string("v4l2src");
    //tmp->nickname = new std::string("camerasrc2");
    //tmp->caps = ;
    //tmp->element = ;
    //tmp->pad = ;
    //tmp->element_prop = new ElementProperty(kSRC, kCAM, "v4l2src", "camerasrc2");;
    element_prop->setWidth(1280);  // FIXME: Any width and height occur ERROR!! 
    element_prop->setHeight(720);
    //tmp->prop = ;
    //tmp->type = kSRC;
    //tmp->sub_type = kCAM;
    tmp->element_prop = element_prop;
    initializeTypeElement(tmp, tmp->element_prop);
    this->_type_element_vector->push_back(tmp);
    tmp = NULL;
  }
}

void freeTypeElementMember(typeElement *type_element)
{
  assert(type_element != NULL);
  if(type_element->name != NULL)
    delete type_element->name;
  if(type_element->nickname != NULL)
    delete type_element->nickname;
  if(type_element->caps != NULL)
    gst_caps_unref(type_element->caps);
}

OPELGstElementTx1::OPELGstElementTx1()
{
  this->main_tee = (typeElement*)malloc(sizeof(typeElement));
  this->v_global_request = new OPELGlobalVectorRequest();
  this->streaming_request = NULL;
	this->delay_request = NULL;
}
OPELGstElementTx1::~OPELGstElementTx1()
{
  for(int i=0; i < this->_type_element_vector->size(); i++)
  {
    freeTypeElementMember((*this->_type_element_vector)[i]);
    delete (*this->_type_element_vector)[i];  
  }
  if(this->pipeline != NULL)
    gst_object_unref(pipeline);

  if(this->main_tee != NULL)
    free(this->main_tee);

  if(this->v_global_request != NULL)
    delete this->v_global_request;
}
/*
OPELGstElementTx1 *OPELGstElementTx1::getInstance(void)
{
  if(opel_gst_element_tx1 == NULL)
    opel_gst_element_tx1 = new OPELGstElementTx1();
  return opel_gst_element_tx1;
}
*/
bool OPELGstElementTx1::OPELGstElementCapFactory(void)
{
  assert(_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  char caps_buffer[256];
  typeElement *src_element = NULL;
  typeElement *conv_element = NULL;
  ElementProperty *prop_element = NULL;
  typeElement *tee_element = NULL;
  typeElement *enc_element = NULL;

  bool ret = true;
#if TARGET_SRC_IS_CAM
  if (this->camera_num == 0)
    src_element = findByElementNickname(this->_type_element_vector, "camerasrc");
  else if (this->camera_num == 1)
    src_element = findByElementNickname(this->_type_element_vector, "camerasrc2");
  else
    return false;

  conv_element = findByElementNicknameNSubType(this->_type_element_vector, "converter",
      kI420);
  tee_element = findByElementName(this->_type_element_vector, "tee");
  enc_element = findByElementNickname(this->_type_element_vector, "h264enc");

  if(!src_element || !conv_element)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

#if OPEL_LOG_VERBOSE
  std::cout <<"find name : " <<  *(src_element->name) << " sub_type : " 
    << src_element->sub_type << std::endl;
  std::cout <<"find name : " <<  *(conv_element->name) << " sub_type : " 
    << conv_element->sub_type << std::endl;
#endif

  prop_element = src_element->element_prop;

#if OPEL_LOG_VERBOSE
  std::cout << " Width : " << prop_element->getWidth() << " Height : " <<
    prop_element->getHeight() << std::endl;
#endif

  /* Poring layer code */
  switch(g_target_type){
    case TX1:
      if (this->camera_num == 0)
        sprintf(caps_buffer, "video/x-raw(memory:NVMM)");
      else if (this->camera_num == 1)
        sprintf(caps_buffer, "video/x-raw");
      else
        return false;

      sprintf(caps_buffer, "%s, format=(string){I420}, "
          "width=(int){%d}, height=(int){%d}", caps_buffer,
          prop_element->getWidth(), prop_element->getHeight());
      break;
    case RPI2_3:
      sprintf(caps_buffer, "video/x-raw");
      break;
    default:
      return false;
  }

  src_element->caps = gst_caps_from_string(caps_buffer);
  if(src_element->caps == NULL)
  {
    OPEL_DBG_ERR("%s : Setting Caps Failed", src_element->name->c_str());
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  switch(g_target_type){
    case TX1:
      conv_element->caps = gst_caps_from_string("video/x-raw(memory:NVMM)");
      break;
    case RPI2_3:
      conv_element->caps = gst_caps_from_string("video/x-raw");
      break;
    default:
      return false;
  }

  if(conv_element->caps == NULL)
  {
    OPEL_DBG_ERR("%s : Setting Caps Failed", conv_element->name->c_str());
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

if(g_target_type == TX1) {
  enc_element->caps = gst_caps_new_simple("video/x-h264",
      "stream-format", G_TYPE_STRING, "avc", NULL);
} else if (g_target_type == RPI2_3) {
  enc_element->caps = gst_caps_new_simple("video/x-h264",NULL);
}
  
#endif /* TARGET_SRC_IS_CAM */

  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstElementPropFactory(void)
{
  assert(this->_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  g_object_set(G_OBJECT(this->pipeline), "message-forward", TRUE, NULL);
  gstElementPropFactory(this->_type_element_vector);
  ////g_object_set(this->bin, "message-forward", TRUE, NULL);

	g_object_set(G_OBJECT(this->c_overlay),
			"time-format", "%Y.%m.%d. %H:%M:%S",
			"font-desc", "Sans, 20",
			"halignment", 2,
			"shaded-background", true,
			NULL);

	g_object_set(G_OBJECT(this->t_overlay),
			"font-desc", "Sans, 15",
			"shaded-background", true,
			NULL);

  typeElement* _enc = 
    findByElementNickname(this->_type_element_vector, "h264enc");

  if (g_target_type == TX1)
    g_object_set(G_OBJECT(_enc->element), "control-rate", (guint)2,
        "bitrate", (guint)2000000, NULL);

  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstPipelineMake(void)
{
  assert(this->_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  GstPadTemplate *tee_src_pad_templ = NULL;
  GstPad *tee_src_pad = NULL;
  GstPad *queue_sink_pad = NULL;
  gboolean ret = false;

  //typeElement* tee =
  //  findByElementName(this->_type_element_vector, "tee");

  typeElement* cam_src;
  if (this->camera_num == 0)
    cam_src = findByElementNickname(this->_type_element_vector, "camerasrc");
  else if (this->camera_num == 1) {
    cam_src = findByElementNickname(this->_type_element_vector, "camerasrc2");
    g_object_set(G_OBJECT(cam_src->element), "device", "/dev/video1", NULL);	// TODO: device name
  }
  else
    return false;

  typeElement* conv =
    findByElementNicknameNSubType(this->_type_element_vector, "converter", kI420);
  typeElement* queue =
    findByElementName(this->_type_element_vector, "queue");

  typeElement* enc = 
    findByElementNickname(this->_type_element_vector, "h264enc");

  //if(!tee || !cam_src || !conv || !queue || !this->main_tee)
  if(!cam_src || !conv || !queue || !this->main_tee)
  {
    OPEL_DBG_ERR("Elements are NULL Erorr");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

#if OPEL_LOG_VERBOSE
//  std::cout <<"find name : " <<  *(tee->name) << " sub_type : " << tee->sub_type << std::endl;
  std::cout <<"find name : " <<  *(cam_src->name) << " sub_type : " << cam_src->sub_type << std::endl;
  std::cout <<"find name : " <<  *(conv->name) << " sub_type : " << conv->sub_type << std::endl;
#endif

  if (this->camera_num == 1)
    g_object_set(G_OBJECT(conv->element), "flip-method", 0, NULL);

  gst_bin_add_many(GST_BIN(this->pipeline), cam_src->element,this->main_tee->element,
      //queue->element, conv->element, tee->element, this->c_overlay, this->t_overlay,
      queue->element, conv->element, this->c_overlay, this->t_overlay, enc->element,
      this->t_conv, this->raw_tee, this->raw_queue, this->h264_tee, NULL);

  ret = gst_element_link_filtered(cam_src->element, this->main_tee->element, cam_src->caps);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link filtered Failed");
    __OPEL_FUNCTION_EXIT__;  
    return false;
  }

  ret = gst_element_link_many(queue->element, conv->element, this->c_overlay,
      this->t_overlay, this->t_conv, this->raw_tee, this->raw_queue, enc->element,
      NULL);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link filtered Failed");
    __OPEL_FUNCTION_EXIT__;  
    return false;
  }

   ret = gst_element_link_filtered(enc->element, this->h264_tee, enc->caps);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link filtered Failed");
    __OPEL_FUNCTION_EXIT__;  
    return false;
  }
/*
  ret = gst_element_link_filtered(this->t_conv, tee->element, conv->caps);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link filtered Failed");
    __OPEL_FUNCTION_EXIT__;  
    return false;
  }
*/
  tee_src_pad_templ = gst_element_class_get_pad_template(
      GST_ELEMENT_GET_CLASS (this->main_tee->element), "src_%u");
  tee_src_pad = gst_element_request_pad(this->main_tee->element, tee_src_pad_templ,
      NULL, NULL);
  this->main_tee->pad = tee_src_pad;
  queue_sink_pad = gst_element_get_static_pad(queue->element, "sink");

  ret = gst_pad_link (tee_src_pad, queue_sink_pad);
  if(ret != GST_PAD_LINK_OK)
  {
    OPEL_DBG_ERR("Tee queue link failed");
    __OPEL_FUNCTION_EXIT__;  
    return false;
  }

  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstElementFactory(void)
{
  assert(this->_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  bool ret = true;
  typeElement *iter = NULL;

  OPEL_GST_ELEMENT_FACTORY_MAKE(this->pipeline, "pipeline", NULL);
  //OPEL_GST_ELEMENT_FACTORY_MAKE(this->bin, "bin", NULL);
  ret = gstElementFactory(this->_type_element_vector); 
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Factory Error");
    __OPEL_FUNCTION_EXIT__;
    return ret; 
  }
  //Main Branch
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->main_tee->element, "tee", NULL);

  // For the text overlay
	OPEL_GST_ELEMENT_FACTORY_MAKE(this->c_overlay, "clockoverlay", NULL);
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->t_overlay, "textoverlay", NULL);
  if (g_target_type == TX1)
    OPEL_GST_ELEMENT_FACTORY_MAKE(this->t_conv, "nvvidconv", NULL);
  else if (g_target_type == RPI2_3)
    OPEL_GST_ELEMENT_FACTORY_MAKE(this->t_conv, "videoconvert", NULL);
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->raw_tee, "tee", NULL);
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->h264_tee, "tee", NULL);
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->raw_queue, "queue", NULL);

  __OPEL_FUNCTION_EXIT__;
  return true;
}


bool OPELRequestTx1::defaultRecordingCapFactory(void)
{
  assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
  __OPEL_FUNCTION_ENTER__;
/*
  typeElement *_enc = findByElementNickname(this->_v_fly_type_element,
      "h264enc");
  if(_enc == NULL)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  switch(g_target_type){
    case TX1:
      _enc->caps = gst_caps_new_simple("video/x-h264",
          "stream-format", G_TYPE_STRING, "avc", NULL);
      break;
    case RPI2_3:
      _enc->caps = gst_caps_new_simple("video/x-h264", NULL);
      break;
    default:
      return false;
  }
*/

  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELRequestTx1::defaultJpegCapFactory(void)
{
  assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
  __OPEL_FUNCTION_ENTER__;
  char caps_buffer[256];
  typeElement *_enc = findByElementNickname(this->_v_fly_type_element,
      "jpegenc");
  if(_enc == NULL)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  sprintf(caps_buffer, "image/jpeg, "
      "width=(int){1920}, height=(int){1080}, framerate=(fraction){1/1}");
  _enc->caps = gst_caps_from_string(caps_buffer);
  if(!_enc->caps)
  {
    OPEL_DBG_ERR("Default JPEG caps is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }
  __OPEL_FUNCTION_EXIT__;
  return true;
}


bool OPELRequestTx1::defaultRecordingPipelineAdd(GstElement *pipeline)
{
  assert(this->_v_type_element != NULL &&
      this->_v_fly_type_element != NULL && pipeline != NULL);
  __OPEL_FUNCTION_ENTER__;
  bool ret = true;

  typeElement *_tee = findByElementName(this->_v_type_element, 
      "tee"); 
  typeElement *_queue = findByElementName(this->_v_fly_type_element, 
      "queue"); 
  //typeElement *_enc = findByElementNickname(this->_v_fly_type_element, 
  //    "h264enc");
  typeElement *_parse = findByElementName(this->_v_fly_type_element,
      "h264parse");
  typeElement *_mux = findByElementNickname(this->_v_fly_type_element, 
      "mp4mux"); 
  typeElement *_sink = findByElementNicknameNSubType(this->_v_fly_type_element,
      "filesink", kREC_SINK); 

  //if(!_tee || !_queue || !_enc || !_mux || !_sink)
  if(!_tee || !_queue || !_parse || !_mux || !_sink)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  gst_bin_add_many(GST_BIN(pipeline), _queue->element, _parse->element,
      _mux->element, _sink->element, NULL);
  ret = gst_element_link_many(_queue->element, _parse->element, _mux->element,
      _sink->element, NULL);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link Many Failed");
    __OPEL_FUNCTION_EXIT__;  
    return ret;
  }
  
  /*
  ret = gst_element_link_many(_queue->element, _enc->element, NULL);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link Many Failed");
    __OPEL_FUNCTION_EXIT__;  
    return ret;
  }

  ret = gst_element_link_filtered(_enc->element, _mux->element, _enc->caps);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link Filtered Failed");
    __OPEL_FUNCTION_EXIT__;  
    return ret;
  }

  ret = gst_element_link(_mux->element, _sink->element); 
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link Failed");
    __OPEL_FUNCTION_EXIT__;  
    return ret;
  }
  */

  __OPEL_FUNCTION_EXIT__;
  return ret;
}

void gstElementPropFactory(std::vector<typeElement*> *_type_element_vector)
{
  assert(_type_element_vector != NULL);

  typeElement *iter = NULL;

  for(int i=0; i<_type_element_vector->size(); i++)
  {
    iter = (*_type_element_vector)[i];
    if(iter->prop != NULL)
      iter->prop->setGstObjectProperty(iter->element);
  }

}

bool gstElementFactory(std::vector<typeElement*>
    *_type_element_vector)
{
  assert(_type_element_vector != NULL);
  typeElement *iter = NULL;
  for(int i=0; i<_type_element_vector->size(); i++)
  {
    iter = (*_type_element_vector)[i];
    OPEL_GST_ELEMENT_FACTORY_MAKE(iter->element,
        stringToGchar(iter->name), NULL);
#if OPEL_LOG_VERBOSE
    g_print("Element Factory : %s\n", stringToGchar(iter->name)); 
#endif
  }

  return true;
}

bool OPELRequestTx1::defaultRecordingElementFactory(const char *file_path)
{
  assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
  __OPEL_FUNCTION_ENTER__;
  
  this->num_iter = OPEL_NUM_DEFAULT_RECORDING_ELE;

#if OPEL_LOG_VERBOSE
  std::cout << "File Path : " << file_path << std::endl;
#endif

  std::vector<typeElement*> _v_original_element(OPEL_NUM_DEFAULT_RECORDING_ELE);
  typeElement *_queue = findByElementName(this->_v_type_element, "queue");
  //typeElement *_enc = findByElementNickname(this->_v_type_element, "h264enc");
  typeElement *_parse = findByElementName(this->_v_type_element, "h264parse");
  typeElement *_mux = findByElementNickname(this->_v_type_element, "mp4mux");
  typeElement *_sink = findByElementNicknameNSubType(this->_v_type_element,
      "filesink", kREC_SINK);

  //if(!_queue || !_enc || !_mux || !_sink)
  if(!_queue || !_parse || !_mux || !_sink)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  ((FileSinkProp*)_sink->prop)->location = file_path;

  _v_original_element[0] = _queue;
  //_v_original_element[1] = _enc;
  _v_original_element[1] = _parse;
  _v_original_element[2] = _mux;
  _v_original_element[3] = _sink;
  //On The Fly Element

  for(int i=0; i<OPEL_NUM_DEFAULT_RECORDING_ELE; i++)
  {
    typeElement *tmp = (typeElement*)malloc(sizeof(typeElement));
    tmp->element_prop = _v_original_element[i]->element_prop;
    initializeTypeElement(tmp, tmp->element_prop);
    this->_v_fly_type_element->push_back(tmp);
    tmp = NULL;
  }
/*
  if(g_target_type == RPI2_3)
  {
    typeElement *_parse = findByElementName(this->_v_type_element, "h264parse");
    if(!_parse)
    {
      OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
      __OPEL_FUNCTION_EXIT__;
      return false;
    }
    
    typeElement *tmp = (typeElement*)malloc(sizeof(typeElement));
    tmp->element_prop = _parse->element_prop;
    initializeTypeElement(tmp, tmp->element_prop);
    this->_v_fly_type_element->push_back(tmp);
    tmp = NULL;
  }
*/
  gstElementFactory(this->_v_fly_type_element);

  gstElementPropFactory(this->_v_fly_type_element);

#if OPEL_LOG_VERBOSE
  for(int i=0; i<this->num_iter; i++)
  {
    std::cout << "name : " <<
      (*this->_v_fly_type_element)[i]->name->c_str() << std::endl;
  }
#endif

  __OPEL_FUNCTION_EXIT__;
  return true; 
}
void OPELgstSyncStateWithParent(std::vector<typeElement*> *_v_ele_property)
{
  assert(_v_ele_property != NULL);
  for(int i=0; i<_v_ele_property->size(); i++)
    if(!gst_element_sync_state_with_parent((*_v_ele_property)[i]->element))
      OPEL_DBG_ERR("Cannot Sync State With Parent");
}

void OPELRequestTx1::defualtSnapshotGstSyncStateWithParent(void)
{
  assert(this->_v_fly_type_element != NULL);
  __OPEL_FUNCTION_ENTER__;
  OPELgstSyncStateWithParent(this->_v_fly_type_element);
  __OPEL_FUNCTION_EXIT__;
}

void OPELRequestTx1::defaultRecordingGstSyncStateWithParent(void)
{
  assert(this->_v_fly_type_element != NULL);
  __OPEL_FUNCTION_ENTER__;
  for(int i=0; i<OPEL_NUM_DEFAULT_RECORDING_ELE; i++)
    if(!gst_element_sync_state_with_parent((*this->_v_fly_type_element)[i]->element))
      OPEL_DBG_ERR("Cannot Sync State With Parent");

  /*
     for(int i=0; i<OPEL_NUM_DEFAULT_RECORDING_ELE; i++)
     gst_element_set_state((*this->_v_fly_type_element)[i]->element, GST_STATE_PLAYING);
   */

  __OPEL_FUNCTION_EXIT__;
}

bool OPELRequestTx1::defaultJpegElementFactory(const char* file_path)
{
  assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
  __OPEL_FUNCTION_ENTER__;

  this->num_iter = OPEL_NUM_DEFAULT_SNAPSHOT_ELE;
  std::vector<typeElement*> _v_original_element(OPEL_NUM_DEFAULT_SNAPSHOT_ELE);
  typeElement *_queue = findByElementName(this->_v_type_element, "queue");
  typeElement *_enc = findByElementNickname(this->_v_type_element, "jpegenc");
  typeElement *_sink = findByElementName(this->_v_type_element,
      "appsink");

  if(!_queue || !_enc  || !_sink)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  //((FileSinkProp*)_sink->prop)->location = file_path;

  _v_original_element[0] = _queue;
  _v_original_element[1] = _enc;
  _v_original_element[2] = _sink;
  for(int i=0; i<OPEL_NUM_DEFAULT_SNAPSHOT_ELE; i++)
  {
    typeElement *tmp = (typeElement*)malloc(sizeof(typeElement));
    tmp->element_prop = _v_original_element[i]->element_prop;
    initializeTypeElement(tmp, tmp->element_prop);
    this->_v_fly_type_element->push_back(tmp);
    tmp = NULL;
  }
  gstElementFactory(this->_v_fly_type_element);
  gstElementPropFactory(this->_v_fly_type_element);

  //#if OPEL_LOG_VERBOSE
  for(int i=0; i<OPEL_NUM_DEFAULT_SNAPSHOT_ELE; i++)
  {
    std::cout << "name : " <<
      (*this->_v_fly_type_element)[i]->name->c_str() << std::endl;
  }
  //#endif

  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELRequestTx1::defaultJpegElementPipelineAdd(GstElement *pipeline)
{
  assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
  __OPEL_FUNCTION_ENTER__;

  bool ret = true;
  typeElement *_tee = findByElementName(this->_v_type_element,
      "tee");
  typeElement *_queue = findByElementName(this->_v_fly_type_element,
      "queue");
  typeElement *_enc = findByElementNickname(this->_v_fly_type_element,
      "jpegenc");
  typeElement *_sink = findByElementName(this->_v_fly_type_element,
      "appsink");

  if(!_tee || !_queue || !_enc  || !_sink)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  this->jpeg_app_sink = _sink;

  gst_bin_add_many(GST_BIN(pipeline), _queue->element, _enc->element,
      _sink->element, NULL);

  ret = gst_element_link_many(_queue->element, _enc->element, NULL);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link Failed");
    __OPEL_FUNCTION_EXIT__;
    return ret;
  }

  ret = gst_element_link_many(_enc->element, _sink->element, NULL);
  if(!ret)
  {
    OPEL_DBG_ERR("Gst Element Link Failed");
    __OPEL_FUNCTION_EXIT__;
    return ret;
  }

  __OPEL_FUNCTION_EXIT__;
  return ret;
}
gboolean onSinkMessage(GstBus *bus, GstMessage *message, gpointer data)
{
  switch(GST_MESSAGE_TYPE (message)){
    case GST_MESSAGE_EOS:
      OPEL_DBG_ERR("Finished Playback\n");
      break;
    case GST_MESSAGE_ERROR:
      OPEL_DBG_ERR("Received Error\n");
      break;
    default:
      break;
  }

  return TRUE;
}
