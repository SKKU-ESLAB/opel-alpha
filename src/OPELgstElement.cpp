#include "OPELgstElement.h"

OPELGstElement* OPELGstElement::opel_gst_element = NULL;


OPELGstElement* OPELGstElement::getInstance(void)
{
  if(opel_gst_element == NULL)
    opel_gst_element = new OPELGstElement();
  return opel_gst_element;
}

static bool typeElementAllocator(const char *name, const char *element_name,
    GstElement **element, int eid, typeElement **type_element_array)
{
   __OPEL_FUNCTION_ENTER__;
   if(!element || !type_element_array)
   {
    __OPEL_FUNCTION_EXIT__; 
     OPEL_DBG_ERR("Element, Type Element Array is NULL"); 
     return false;
   }
   type_element_array[eid]->name = name;
   type_element_array[eid]->element_name = element_name;
   type_element_array[eid]->element = element[eid];
   type_element_array[eid]->type = eid;
   __OPEL_FUNCTION_EXIT__;
   return true;
}
    
bool OPELGstElement::OPELGstPipelineMake(void)
{
  

}
bool OPELGstElement::OPELGstElementFactory(void)
{ 
  __OPEL_FUNCTION_ENTER__;

   OPEL_GST_PIPELINE_NEW(this->element_array, kPIPELINE, "pipeline");
   OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kSRC, "nvcamerasrc", "src");  
   OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kCONV, "videoconvert", "convert");  
   OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kENC, "omxh264enc", "encorder");  
   OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kMUX, "mp4mux", "mux");  
   OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kSINK, "filesink", "filesink");  
   for(int pipe_idx=0; pipe_idx<NUM_OF_GST_ELEMENT; pipe_idx++) 
    if(this->element_array[pipe_idx] == NULL)
    {
      __OPEL_FUNCTION_EXIT__;
      return false;
    }
   typeElementAllocator("pipeline", "pipeline", this->element_array,
       kPIPELINE, this->type_element_array);
   typeElementAllocator("src", "nvcamerasrc", this->element_array, 
       kSRC, this->type_element_array);
   typeElementAllocator("convert", "videoconvert", this->element_array,
       kCONV, this->type_element_array);
   typeElementAllocator("encorder", "omxh264enc", this->element_array,
       kENC, this->type_element_array);
   typeElementAllocator("mux", "mp4mux", this->element_array, 
       kMUX, this->type_element_array);
   typeElementAllocator("filesink", "filesink", this->element_array,
       kSINK, this->type_element_array);
   for(int pipe_idx=0; pipe_idx<NUM_OF_GST_TYPE_ELEMENT; pipe_idx++)
    if(this->type_element_array == NULL)
    {
      __OPEL_FUNCTION_EXIT__;
      return false;
    }
  __OPEL_FUNCTION_EXIT__;
  return true;
}

OPELGstElement::OPELGstElement()
{
  this->element_array = (GstElement**)malloc(sizeof(GstElement*) * NUM_OF_GST_ELEMENT);

  this->type_element_array = (typeElement**)malloc(sizeof(typeElement*) * NUM_OF_GST_ELEMENT);

  for(int pipe_idx=0; pipe_idx<NUM_OF_GST_ELEMENT; pipe_idx++)
    this->type_element_array[pipe_idx] = (typeElement*)malloc(sizeof(typeElement));
}


OPELGstElement::~OPELGstElement()
{
  for(int pipe_idx=0; pipe_idx<NUM_OF_GST_ELEMENT; pipe_idx++)
    OPEL_GST_ELEMENT_UNREF(this->element_array, pipe_idx);
      
  free(this->element_array);

  for(int pipe_idx=0; pipe_idx<NUM_OF_GST_TYPE_ELEMENT; pipe_idx++)
  {   
    if(this->type_element_array[pipe_idx]->caps != NULL)
      gst_caps_unref(type_element_array[pipe_idx]->caps);
    free(this->type_element_array[pipe_idx]);
  }  
  free(this->type_element_array); 

  if(this->main_loop != NULL)
    g_main_loop_unref(this->main_loop);
   
}

