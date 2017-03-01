#ifndef OPEL_GST_ELEMENT_H
#define OPEL_GST_ELEMENT_H
#include "OPELcamProperty.h"
#include <gst/gst.h>
#include <stdlib.h>
#include <string>
#include "OPELconfig.h"
#include "OPELcamRequest.h"
#include <dbus/dbus.h>

//#define TARGET_SRC_IS_CAM 1

#define OPEL_GST_ELEMENT_LINK_FILTERED(element, eid_1, eid_2, cap) \
  do{ \
    if(element != NULL && cap != NULL) \
      gst_element_link_filtered(element[eid_1], element[eid_2], cap); \
    else \
      OPEL_DBG_ERR("Element Array or GstCap is NULL"); \
  } while(0)

#define OPEL_G_OBJECT_SET(element, ...) \
  do{ \
    if(element != NULL) \
      g_object_set(G_OBJECT(element), __VA_ARGS__); \
    else \
      OPEL_DBG_ERR("GstElement is NULL"); \
  } while(0)

#define OPEL_GST_CAPS_NEW_SIMPLE(caps, ...) \
  do{ \
    if(caps != NULL) \
      caps = gst_caps_new_simple(__VA_ARGS__); \
    else \
      OPEL_DBG_ERR("Caps Array is NULL"); \
  } while(0)

#define OPEL_GST_ELEMENT_FACTORY_MAKE(element, name, nickname) \
  do{ \
    element = gst_element_factory_make(name, nickname); \
  } while(0)

#define OPEL_GST_PIPELINE_NEW(element, eid, name) \
  do{ \
    if(element != NULL && eid == 0) \
      element[eid] = gst_pipeline_new(name); \
    else \
      OPEL_DBG_ERR("Gstreamer Pipeline Make Error"); \
  } while(0)

#define OPEL_GST_ELEMENT_UNREF(element, eid) \
  do{ \
    if(element != NULL){ \
      gst_object_unref(element[eid]); \
      element[eid] = NULL; \
    } \
  } while(0)

typedef struct {
  std::string *name;
  std::string *nickname;
  GstCaps *caps;
  GstElement *element;
  GstPad *pad;
  ElementProperty *element_prop;
  Property *prop;
  unsigned type;
  unsigned sub_type;
} typeElement;


class OPELRequest {
  public:
    OPELRequest();
    ~OPELRequest();
    virtual bool defaultRecordingElementFactory(const char *file_path) = 0;
    virtual bool defaultRecordingPipelineAdd(GstElement *pipeline) = 0;
    virtual bool defaultRecordingCapFactory(void) = 0;

    virtual bool defaultJpegElementFactory(const char* file_path) = 0;
    virtual bool defaultJpegElementPipelineAdd(GstElement *pipeline) = 0;
    virtual bool defaultJpegCapFactory(void) = 0;

    std::vector<typeElement*>* getTypeElementVector(void) const
    { return this->_v_type_element; }
    void setTypeElementVector(std::vector<typeElement*>
        *__v_type_element)
    { this->_v_type_element = __v_type_element; }
    std::vector<typeElement*>* getFlyTypeElementVector(void) const
    { return this->_v_fly_type_element; }
    void setSrcPad(GstPad *_src_pad)
    { this->src_pad = _src_pad; }
    GstPad* getSrcPad(void) const
    { return this->src_pad; }

    dbusRequest *getMsgHandle(void) const
    { return this->msg_handle; }
    void setMsgHandle(dbusRequest *_msg_handle)
    { this->msg_handle = _msg_handle; }

    unsigned getNumIter()
    { return this->num_iter; }
    void setNumIter(unsigned _num_iter)
    { this->num_iter = _num_iter; }

    DBusMessage* getDBusMessage(void)
    { return this->reply; }
    DBusConnection* getDBusConnection(void)
    { return this->conn; }
    void setDBusMessage(DBusMessage *_reply)
    { this->reply = _reply; }
    void setDBusConnection(DBusConnection *_conn)
    { this->conn = _conn; }

    dbusStreamingRequest* getStreamingRequest(void)
    { return this->streaming_handle; } 
    void setStreamingRequest(dbusStreamingRequest* _streaming_handle)
    { this->streaming_handle = _streaming_handle; }

  protected:
    std::vector<typeElement*> *_v_type_element;
    std::vector<typeElement*> *_v_fly_type_element;
    GstPad *src_pad;
    GstPadTemplate *src_templ;
    dbusRequest *msg_handle;
    dbusStreamingRequest *streaming_handle;
    unsigned num_iter;
    DBusMessage *reply;
    DBusConnection *conn;
};


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
typeElement* findByElementName(std::vector<typeElement*> *_v_element, 
    const char *_str);
typeElement* findByElementNameNSubType(std::vector<typeElement*> *_v_element, 
    const char *_str, unsigned _sub_type);


#endif /* OPEL_GST_ELEMENT_H */
