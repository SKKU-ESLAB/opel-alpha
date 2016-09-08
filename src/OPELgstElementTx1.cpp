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

static void initializeTypeElement(typeElement *type_element, 
    ElementProperty *element_property)
{
  assert(type_element != NULL && element_property != NULL);

  type_element->name = new std::string(element_property->getElementName());
  type_element->nickname = new std::string(element_property->getElementNickName());
  type_element->element_prop = element_property;
  type_element->type = element_property->getElementType();
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
// buggy
//   if(type_element->caps != NULL)
//     gst_caps_unref(type_element->caps);
   if(type_element->element != NULL)
     gst_object_unref(GST_OBJECT(type_element->element));
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
/*
  if(!is_rtsp_src){ 
  OPEL_GST_CAPS_NEW_SIMPLE(caps_array, kSRC, "video/x-raw(memory:NVMM)", "width",
      G_TYPE_INT, 1920, "height", G_TYPE_INT, 1080, "format", G_TYPE_STRING, 
      "I420", "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
  typeElementCapAllocator(kSRC, this->type_element_array, caps_array[kSRC]);
  }

  OPEL_GST_CAPS_NEW_SIMPLE(caps_array, kCONV, "video/x-raw(memory:NVMM)", "format", 
      G_TYPE_STRING, "I420", NULL);
  typeElementCapAllocator(kCONV, this->type_element_array, caps_array[kCONV]);

  OPEL_GST_CAPS_NEW_SIMPLE(caps_array, kENC, "video/x-h264", "stream-format", 
      G_TYPE_STRING, "avc", NULL);
  typeElementCapAllocator(kENC, this->type_element_array, caps_array[kENC]);
  
  
  if(!caps_array[kSRC] || !caps_array[kCONV] || !caps_array[kENC])
  {
    OPEL_DBG_ERR("Capabilities setting error");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }
  */
  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstElementPropFactory(void)
{
  assert(_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
 /*
 if(!is_rtsp_src){
 OPEL_G_OBJECT_SET(element_array, kSRC, "fpsRange", "30.0 30.0", NULL);
 }
 OPEL_G_OBJECT_SET(element_array, kCONV, "flip-method", 2, NULL);
 OPEL_G_OBJECT_SET(element_array, kENC, "bitrate", 8000000, NULL);
 OPEL_G_OBJECT_SET(element_array, kSINK, "location", "~/recording/recording_data", NULL);
*/
 __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstPipelineMake(void)
{
  assert(_type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;



  
  /*
  gst_bin_add_many(GST_BIN(element_array[kPIPELINE]), element_array[kSRC],
      element_array[kCONV], element_array[kENC], element_array[kMUX],
      element_array[kSINK], NULL);
  ret = gst_element_link_many(element_array[kSRC],
      element_array[kCONV], element_array[kENC], element_array[kMUX],
      element_array[kSINK], NULL);
  if(ret == false)
  {
    OPEL_DBG_ERR("Gstreamer element Linking error");
    __OPEL_FUNCTION_EXIT__;
    return ret;
  }
  for(int pipe_idx=0; pipe_idx<NUM_OF_GST_ELEMENT; pipe_idx++)
  {
    int next_element = pipe_idx+1;
    if(type_element_array[pipe_idx]->caps != NULL)
    { 
      OPEL_GST_ELEMENT_LINK_FILTERED(element_array, pipe_idx, next_element, 
          type_element_array[pipe_idx]->caps);
    }
  }
  */

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
//    g_print("Element Factory : %s\n", stringToGchar(iter->name)); 
  }

  __OPEL_FUNCTION_EXIT__;
  return true;
}

