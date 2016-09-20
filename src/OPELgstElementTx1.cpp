#include "OPELgstElementTx1.h"
OPELGstElementTx1 *OPELGstElementTx1::opel_gst_element_tx1 = NULL; 


void printTypeElement(std::vector<typeElement*> *_type_element_vector)
{
   assert(_type_element_vector != NULL);

   for(int i=0; i<_type_element_vector->size(); i++)
   {
     std::cout << *((*_type_element_vector)[i]->name) << std::endl;
     std::cout << *((*_type_element_vector)[i]->nickname) << std::endl;
   }
}
static void inline setTypeProperty(unsigned _sub_type, 
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
    default:
        OPEL_DBG_ERR("Invalid Sub-Type");
      break;

  }
}
static void initializeTypeElement(typeElement *type_element, 
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
}

OPELGstElementTx1 *OPELGstElementTx1::getInstance(void)
{
  if(opel_gst_element_tx1 == NULL)
    opel_gst_element_tx1 = new OPELGstElementTx1();
  return opel_gst_element_tx1;
}

bool OPELGstElementTx1::OPELGstElementCapFactory(void)
{
  assert(_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  char caps_buffer[256];
  typeElement *src_element = NULL;
  typeElement *conv_element = NULL;
  ElementProperty *prop_element = NULL;
  typeElement *tee_element = NULL;
  
  bool ret = true;

#if TARGET_SRC_IS_CAM
  src_element = findByElementName(this->_type_element_vector, "nvcamerasrc");
  conv_element = findByElementNameNSubType(this->_type_element_vector, "nvvidconv",
      kI420);
  tee_element = findByElementName(this->_type_element_vector, "tee");

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

#if OPEL_LOG_VERBOSE
   OPEL_DBG_ERR("Obtained request pad %s for %s", gst_pad_get_name(src_element->pad), 
       src_element->name->c_str());
#endif

   sprintf(caps_buffer, "video/x-raw(memory:NVMM), format=(string){I420}, \
       width=(int){%d}, height=(int){%d}",
       prop_element->getWidth(), prop_element->getHeight());

   src_element->caps = gst_caps_from_string(caps_buffer);
   if(src_element->caps == NULL)
   {
     OPEL_DBG_ERR("%s : Setting Caps Failed", src_element->name->c_str());
     __OPEL_FUNCTION_EXIT__;
     return false;
   }
   conv_element->caps = gst_caps_from_string("video/x-raw(memory:NVMM)");

  if(conv_element->caps == NULL)
  {
    OPEL_DBG_ERR("%s : Setting Caps Failed", conv_element->name->c_str());
    __OPEL_FUNCTION_EXIT__;
    return false;
  }
       
#endif
  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstElementPropFactory(void)
{
  assert(_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  
  typeElement *iter = NULL;
  
  for(int i=0; i<this->_type_element_vector->size(); i++)
  {
    iter = (*this->_type_element_vector)[i];
    if(iter->prop != NULL)
      iter->prop->setGstObjectProperty(iter->element);
  }
  
  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstPipelineMake(void)
{
  assert(_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  gboolean ret = false;
  typeElement* tee = 
    findByElementName(this->_type_element_vector, "tee");
  typeElement* cam_src = 
    findByElementName(this->_type_element_vector, "nvcamerasrc");
  typeElement* conv = 
    findByElementNameNSubType(this->_type_element_vector, "nvvidconv", kI420);
  typeElement* queue = 
     findByElementName(this->_type_element_vector, "queue");


#if OPEL_LOG_VERBOSE
  std::cout <<"find name : " <<  *(tee->name) << " sub_type : " << tee->sub_type << std::endl;
  std::cout <<"find name : " <<  *(cam_src->name) << " sub_type : " << cam_src->sub_type << std::endl;
  std::cout <<"find name : " <<  *(conv->name) << " sub_type : " << conv->sub_type << std::endl;
#endif

  gst_bin_add_many(GST_BIN(this->pipeline), cam_src->element, 
      conv->element, tee->element, NULL);

  ret = gst_element_link_filtered(cam_src->element, conv->element, cam_src->caps);
  if(!ret)
  {
     OPEL_DBG_ERR("Gst Element Link filtered Failed");
     __OPEL_FUNCTION_EXIT__;  
     return false;
  }
  ret = gst_element_link_filtered(conv->element, tee->element, conv->caps);
  if(!ret)
  {
     OPEL_DBG_ERR("Gst Element Link filtered Failed");
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
  
  typeElement *iter = NULL;
  
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->pipeline, "pipeline", NULL);
  
  for(int i=0; i<this->_type_element_vector->size(); i++)
  {
    iter = (*this->_type_element_vector)[i];
    OPEL_GST_ELEMENT_FACTORY_MAKE(iter->element, 
        stringToGchar(iter->name), stringToGchar(iter->nickname));
#if OPEL_LOG_VERBOSE
    g_print("Element Factory : %s\n", stringToGchar(iter->name)); 
#endif
  }
  
  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstElementRecordingCapFactory(void)
{
  assert(this->_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  
  typeElement *_enc = findByElementName(this->_type_element_vector, "omxh264enc"); 

  if(!_enc)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  _enc->caps = gst_caps_new_simple("video/x-h264", 
      "stream-format", G_TYPE_STRING, "avc", NULL);

  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstElementRecordingPipelineMake(void)
{
  assert(this->_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  bool ret = true;
 
  typeElement *_tee = findByElementName(this->_type_element_vector, "tee"); 
  typeElement *_queue = findByElementName(this->_type_element_vector, "queue"); 
  typeElement *_enc = findByElementName(this->_type_element_vector, "omxh264enc"); 
  typeElement *_mux = findByElementName(this->_type_element_vector, "mp4mux"); 
  typeElement *_sink = findByElementNameNSubType(this->_type_element_vector, 
      "filesink", kREC_SINK); 
 
  if(!_tee || !_queue || !_enc || !_mux || !_sink)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  gst_bin_add_many(GST_BIN(this->pipeline), _queue->element, _enc->element,
      _mux->element, _sink->element, NULL);

  ret = gst_element_link_many(_tee->element, _queue->element, _enc->element, NULL);
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
  __OPEL_FUNCTION_EXIT__;
  return true;
}


