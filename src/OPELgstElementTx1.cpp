#include "OPELgstElementTx1.h"
OPELGstElementTx1 *OPELGstElementTx1::opel_gst_element_tx1 = NULL; 

OPELGstElementTx1::OPELGstElementTx1() : is_rtsp_src(false) 
{
 this->caps_array = (GstCaps**)malloc(
     sizeof(GstCaps*) * NUM_OF_GST_ELEMENT); 
 if(this->caps_array == NULL)
   OPEL_DBG_ERR("Capabilites array allocation failed");

}
OPELGstElementTx1::~OPELGstElementTx1()
{
  if(this->main_loop != NULL)
    g_main_loop_unref(this->main_loop);
  
  if(this->caps_array != NULL)
    for(int pipe_idx=0; pipe_idx<NUM_OF_GST_ELEMENT; pipe_idx++)
    {   
      gst_caps_unref(caps_array[pipe_idx]);
      caps_array[pipe_idx] = NULL;
    }
  free(this->caps_array);
}

OPELGstElementTx1 *OPELGstElementTx1::getInstance(void)
{
  if(opel_gst_element_tx1 == NULL)
    opel_gst_element_tx1 = new OPELGstElementTx1();
  return opel_gst_element_tx1;
}

bool OPELGstElementTx1::OPELGstElementCapFactory(void)
{
  __OPEL_FUNCTION_ENTER__;
  if(this->caps_array == NULL)
  {
    OPEL_DBG_ERR("Capabilites array is null");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

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
  
  __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstElementPropFactory(void)
{
 __OPEL_FUNCTION_ENTER__;

 if(!is_rtsp_src){
 OPEL_G_OBJECT_SET(element_array, kSRC, "fpsRange", "30.0 30.0", NULL);
 }
 OPEL_G_OBJECT_SET(element_array, kCONV, "flip-method", 2, NULL);
 OPEL_G_OBJECT_SET(element_array, kENC, "bitrate", 8000000, NULL);
 OPEL_G_OBJECT_SET(element_array, kSINK, "location", "~/recording/recording_data", NULL);

 __OPEL_FUNCTION_EXIT__;
  return true;
}

bool OPELGstElementTx1::OPELGstPipelineMake(void)
{
  __OPEL_FUNCTION_ENTER__;
  gboolean ret=true;
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
  return true;
  __OPEL_FUNCTION_EXIT__;
}

bool OPELGstElementTx1::OPELGstElementFactory(void)
{
  __OPEL_FUNCTION_ENTER__;
  const char** element_name = element_name_tx1;
  if(this->is_rtsp_src)
     element_name = element_name_tx1_rtsp;

  
  OPEL_GST_PIPELINE_NEW(this->element_array, kPIPELINE, 
      element_name[kPIPELINE]);
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kSRC, 
      element_name[kSRC], "src");
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kCONV, 
      element_name[kCONV], "convert");
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kENC, 
      element_name[kENC], "encorder");
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kMUX, 
      element_name[kMUX], "mux");
  OPEL_GST_ELEMENT_FACTORY_MAKE(this->element_array, kSINK, 
      element_name[kSINK], "filesink");
  for(int pipe_idx=0; pipe_idx<NUM_OF_GST_ELEMENT; pipe_idx++)
    if(this->element_array[pipe_idx] == NULL)
    {
      __OPEL_FUNCTION_EXIT__;
      return false;
    }

  typeElementAllocator("pipeline", element_name_tx1[kPIPELINE], this->element_array,
      kPIPELINE, this->type_element_array);
  typeElementAllocator("src", element_name[kSRC], this->element_array,
      kSRC, this->type_element_array);
  typeElementAllocator("convert", element_name[kCONV], this->element_array,
      kCONV, this->type_element_array);
  typeElementAllocator("encorder", element_name[kENC], this->element_array,
      kENC, this->type_element_array);
  typeElementAllocator("mux", element_name[kMUX], this->element_array,
      kMUX, this->type_element_array);
  typeElementAllocator("filesink", element_name[kSINK], this->element_array,
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

void OPELGstElementTx1::setIsRtspSrc(bool _is_rtsp_src)
{
  this->is_rtsp_src = _is_rtsp_src;
}
bool OPELGstElementTx1::getIsRtspSrc(void) const
{
  return this->is_rtsp_src; 
}
