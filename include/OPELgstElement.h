#ifndef OPEL_GST_ELEMENT_H
#define OPEL_GST_ELEMENT_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <stdlib.h>

#include "OPELdbugLog.h"


#define TARGET_BOARD_TX1 1

#define NUM_OF_GST_ELEMENT_TX1 6

#ifdef TARGET_BOARD_TX1
#define NUM_OF_GST_ELEMENT NUM_OF_GST_ELEMENT_TX1
#endif /*TARGET_BOARD_TX1*/

#define NUM_OF_GST_TYPE_ELEMENT NUM_OF_GST_ELEMENT

#define OPEL_GST_ELEMENT_FACTORY_MAKE(element, eid, name, nickname) \
  do{ if(element != NULL) \
    element[eid] = gst_element_factory_make(name, nickname); \
    else \
    OPEL_DBG_ERR("Element Array is NULL"); \
  }while(0)

#define OPEL_GST_PIPELINE_NEW(element, eid, name) \
  do{ if(element != NULL && eid == 0) \
    element[eid] = gst_pipeline_new(name); \
    else \
    OPEL_DBG_ERR("Gstreamer Pipeline Make Error");\
  }while(0)

#define OPEL_GST_ELEMENT_UNREF(element, eid) \
  do{ if(element != NULL){ \
    gst_object_unref(element[eid]); \
    element[eid] = NULL; \
  } \
  }while(0)
    

typedef struct {
  const char *name;
  const char *element_name;
  GstCaps *caps;
  GstElement *element;
  unsigned type;
}typeElement;

typedef enum{
  kPIPELINE = 0,
  kSRC,
  kCONV,
  kENC, 
  kMUX, 
  kSINK,
}elementId;

class OPELGstElement 
{
  public:
    ~OPELGstElement();
    OPELGstElement();
    virtual bool OPELGstElementFactory(void) = 0; 
    virtual bool OPELGstPipelineMake(void) = 0;
    virtual void OPELGstElementCapFactory(void) = 0;
  protected: 
    GstElement** element_array;
    typeElement** type_element_array;

    GMainLoop *main_loop;

    GstCaps *caps_src;
    GstCaps *caps_src_to_conv;
    GstCaps *caps_conv_to_enc;

    GstBus *bus;
    guint bus_watch_id; 

};

extern bool typeElementAllocator(const char *name, const char *element_name,
    GstElement **element, int eid, typeElement **type_element_array);

#endif /* OPEL_GST_ELEMENT_H */
