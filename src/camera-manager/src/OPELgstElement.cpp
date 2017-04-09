#include "OPELcommonUtil.h"
#include "OPELgstElement.h"

////GstElement *OPELGstElement::pipeline = NULL;
//OPELGstElement *OPELGstElement::opel_gst_element_bin[2];

extern bool typeElementAllocator(const char *name, const char *element_name,
    GstElement **element, int eid, typeElement **type_element_array)
{
  __OPEL_FUNCTION_ENTER__;
  if(element == NULL || type_element_array == NULL)
  {
    __OPEL_FUNCTION_EXIT__;
    OPEL_DBG_ERR("Element, Type Element Array is NULL");
    return false;
  }
  __OPEL_FUNCTION_EXIT__;
  return true;
}

extern bool typeElementCapAllocator(unsigned eid,
    typeElement **type_element_array, GstCaps *cap)
{
  __OPEL_FUNCTION_ENTER__;
  if(type_element_array == NULL || cap == NULL)
  {
    __OPEL_FUNCTION_EXIT__;
    OPEL_DBG_ERR("Element, Type Element Array is NULL");
    return false;
  }

  __OPEL_FUNCTION_EXIT__;
  return true;
} 
/*
GstElement* OPELGstElement::getPipeline(void)
{ return pipeline; }

bool OPELGstElement::makePipeline(void) {
  OPEL_GST_ELEMENT_FACTORY_MAKE(pipeline, "pipeline", NULL);
  //g_object_set(this->pipeline, "message-forward", TRUE, NULL); // TODO: Need??
}
*/
/*
OPELGstElement* OPELGstElement::getOPELGstElementBin(unsigned camera_num) {
  if (camera_num == 0 || camera_num == 1)
    return opel_gst_element_bin[camera_num];
  else
    return NULL;
}
*/
OPELGstElement::OPELGstElement()
{
  this->_type_element_vector = new std::vector<typeElement*>();
}

OPELGstElement::~OPELGstElement()
{
  assert(this->_type_element_vector != NULL);
  delete this->_type_element_vector;
}

typeElement* findByElementName(std::vector<typeElement*> *_v_element,
    const char *_str)
{
  assert(_str != NULL && _v_element != NULL);
  std::string target = charToString(_str);
  typeElement *iter = NULL;
  for(int i=0; i < _v_element->size(); i++)
  {
    iter = (*_v_element)[i];
    if(target == *(iter->name))
      return iter;
  }
  OPEL_DBG_ERR("Cannot Find Element By Element Name");
  return NULL;
}

typeElement* findByElementNickname(std::vector<typeElement*> *_v_element,
    const char *_str)
{
  assert(_str != NULL && _v_element != NULL);
  std::string target = charToString(_str);
  typeElement *iter = NULL;
  for(int i=0; i < _v_element->size(); i++)
  {
    iter = (*_v_element)[i];
    if(target == *(iter->nickname))
      return iter;
  }
  OPEL_DBG_ERR("Cannot Find Element By Element Nickname");
  return NULL;
}

typeElement* findByElementNameNSubType(std::vector<typeElement*> *_v_element, 
    const char *_str, unsigned _sub_type)
{
  assert(_str != NULL && _v_element != NULL);
  std::string target = charToString(_str);
  typeElement *iter = NULL;
  for(int i=0; i < _v_element->size(); i++)
  {
    iter = (*_v_element)[i];
    if((target == *(iter->name)) && (iter->sub_type == _sub_type)) 
      return iter;
  }
  OPEL_DBG_ERR("Cannot Find Element By Element Name and Sub Type");
  return NULL;
}

typeElement* findByElementNicknameNSubType(std::vector<typeElement*> *_v_element, 
    const char *_str, unsigned _sub_type)
{
  assert(_str != NULL && _v_element != NULL);
  std::string target = charToString(_str);
  typeElement *iter = NULL;
  for(int i=0; i < _v_element->size(); i++)
  {
    iter = (*_v_element)[i];
    if((target == *(iter->nickname)) && (iter->sub_type == _sub_type)) 
      return iter;
  }
  OPEL_DBG_ERR("Cannot Find Element By Element Name and Sub Type");
  return NULL;
}

OPELRequest::OPELRequest()
{
  this->_v_fly_type_element = new std::vector<typeElement*>();
}

OPELRequest::~OPELRequest()
{
  delete this->_v_fly_type_element;
}