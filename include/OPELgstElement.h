#ifndef OPEL_GST_ELEMENT_H
#define OPEL_GST_ELEMENT_H
#include "OPELcamProperty.h"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <stdlib.h>
#include <string>

//#include "OPELdbugLog.h"


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

#define OPEL_GST_ELEMENT_FACTORY_MAKE(element, name, nickname) \
  do{ \
    element = gst_element_factory_make(name, nickname); \
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
  std::string *name;
  std::string *nickname;
  GstCaps *caps;
  GstElement *element;
  ElementProperty *element_prop;
  unsigned type;
}typeElement;


class OPELGstElement 
{
  public:
    ~OPELGstElement();
    OPELGstElement();
    virtual bool OPELGstElementFactory(void) = 0; 
    virtual bool OPELGstPipelineMake(void) = 0;
    virtual bool OPELGstElementCapFactory(void) = 0;
    virtual bool OPELGstElementPropFactory(void) = 0;
    virtual void setElementPropertyVector(std::vector<ElementProperty*> 
        *__v_element_property) = 0;
    std::vector<typeElement*>* getTypeElementVector(void) 
    { return this-> _type_element_vector; }

  protected: 
    std::vector<ElementProperty*> *_v_element_property;
    std::vector<typeElement*> *_type_element_vector;
};


extern bool typeElementAllocator(const char *name, const char *element_name,
    GstElement **element, int eid, typeElement **type_element_array);
extern bool typeElementCapAllocator(unsigned eid,
    typeElement **type_element_array, GstCaps *cap);
#endif /* OPEL_GST_ELEMENT_H */
