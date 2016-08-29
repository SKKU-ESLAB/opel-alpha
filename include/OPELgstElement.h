#ifndef OPEL_GST_ELEMENT_H
#define OPEL_GST_ELEMENT_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <stdlib.h>
#include <string>

#include "OPELdbugLog.h"


#define TARGET_BOARD_TX1 1

#define NUM_OF_GST_ELEMENT_TX1 6

#ifdef TARGET_BOARD_TX1
#define NUM_OF_GST_ELEMENT NUM_OF_GST_ELEMENT_TX1
#endif /*TARGET_BOARD_TX1*/

#define NUM_OF_GST_TYPE_ELEMENT NUM_OF_GST_ELEMENT

#define OPEL_GST_ELEMENT_LINK_FILTERED(element, eid_1, eid_2, cap) \
  do{ if(element != NULL && cap != NULL) \
       gst_element_link_filtered(element[eid_1], element[eid_2], cap); \
    else \
    OPEL_DBG_ERR("Element Array or GstCap is NULL"); \
  }while(0)

#define OPEL_G_OBJECT_SET(element, eid, ...) \
  do{ if(element != NULL) \
      g_object_set(G_OBJECT(element[eid]), __VA_ARGS__); \
    else \
    OPEL_DBG_ERR("Element Array is NULL"); \
  }while(0)


#define OPEL_GST_CAPS_NEW_SIMPLE(caps, eid, ...) \
  do{ if(caps != NULL) \
    caps[eid] = gst_caps_new_simple(__VA_ARGS__); \
    else \
    OPEL_DBG_ERR("Caps Array is NULL"); \
  }while(0)

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
    virtual bool OPELGstElementCapFactory(void) = 0;
    virtual bool OPELGstElementPropFactory(void) = 0;
  protected: 
    GstElement** element_array;
    typeElement** type_element_array;
};

class OPELGstElementCapsProps
{
  public:
    OPELGstElementCapsProps();
    ~OPELGstElementCapsProps();
    GstCaps* getGstCaps(void);
    void setGstCaps(GstCaps  *_caps);
    unsigned getWidth(void);
    void setWidth(unsigned _width);
    unsigned getHeight(void);
    void setHeight(unsigned _height);
    unsigned getFps(void);
    void setFps(unsigned _fps);
    std::string getPixelFormat(void);
    void setPixelFormat(std::string _pixel_format);
    std::string getStreamFormat(void);
    void setStreamFormat(std::string _stream_format); 
  private:
/*Caps*/
    GstCaps *caps;
    elementId type;
    unsigned width;
    unsigned height;
    unsigned fps; 
    std::string pixel_format;
    std::string stream_format;
/*Properties*/

};


extern bool typeElementAllocator(const char *name, const char *element_name,
    GstElement **element, int eid, typeElement **type_element_array);
extern bool typeElementCapAllocator(unsigned eid,
    typeElement **type_element_array, GstCaps *cap);
#endif /* OPEL_GST_ELEMENT_H */
