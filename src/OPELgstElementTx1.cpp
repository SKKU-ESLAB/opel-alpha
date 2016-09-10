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
    case kDEFAULT:
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
    else
    {
      std::cout << "Element Name : " <<  *iter->name << std::endl;
    }

  }
 
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

