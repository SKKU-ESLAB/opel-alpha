#include "OPELgstElementTx1.h"
OPELGstElementTx1 *OPELGstElementTx1::opel_gst_element_tx1 = NULL; 

OPELGstElementTx1::OPELGstElementTx1()
{

}
OPELGstElementTx1::~OPELGstElementTx1()
{

}

OPELGstElementTx1 *OPELGstElementTx1::getInstance(void)
{
  __OPEL_FUNCTION_ENTER__;
  if(opel_gst_element_tx1 == NULL)
    opel_gst_element_tx1 = new OPELGstElementTx1();
  __OPEL_FUNCTION_EXIT__;
  return opel_gst_element_tx1;
}

void OPELGstElementTx1::OPELGstElementCapFactory(void)
{
  __OPEL_FUNCTION_ENTER__;


  __OPEL_FUNCTION_EXIT__;
}
bool OPELGstElementTx1::OPELGstPipelineMake(void)
{
  __OPEL_FUNCTION_ENTER__;


  return true;
  __OPEL_FUNCTION_EXIT__;
}
bool OPELGstElementTx1::OPELGstElementFactory(void)
{
  __OPEL_FUNCTION_ENTER__;

  OPEL_GST_PIPELINE_NEW(this->element_array, kPIPELINE, "pipeline");
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kSRC, "nvcamerasrc", "src");
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kCONV, "nvvidconv", "convert");
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
