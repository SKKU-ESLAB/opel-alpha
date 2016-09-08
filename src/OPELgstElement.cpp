#include "OPELgstElement.h"
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

OPELGstElement::OPELGstElement()
{
  this->_type_element_vector = new std::vector<typeElement*>(); 
}

OPELGstElement::~OPELGstElement()
{
  delete this->_type_element_vector;
}

