#ifndef OPEL_CAMERA_PROPERTY_H
#define OPEL_CAMERA_PROPERTY_H
#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <fstream>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <gst/gst.h>
#include "OPELdbugLog.h"

typedef enum _elementType{
   kSRC = 0,
   kTEE,
   kQUEUE,
   kCONV,
   kENC,
   kMUX,
   kSINK,
}elementType;

typedef enum _subElementType{
  kCAM = 0,
  kRTSP,
  kI420,
  kBGR,
  kH264,
  kNVJPEG,
  kMP4,
  kREC_SINK,
  kJPEG_SINK,
  kUDP_SINK,
  kNO_PROP,
	kAPP_SINK,
}subElementType;


class Property
{
  public:
    virtual void setGstObjectProperty(GstElement *element) = 0;
};

class EncorderProp : public Property 
{
  public:
     friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("quality_level", this->quality_level);
      ar & make_nvp("bitrate", this->bitrate);
    }
    virtual void setGstObjectProperty(GstElement *element)
    {
      assert(element != NULL);
      g_object_set(G_OBJECT(element), this->n_quality_level, this->quality_level, 
          this->n_bitrate, this->bitrate, NULL);
    }
    static const char *n_quality_level;
    static const char *n_bitrate;
    unsigned quality_level;
    unsigned bitrate;
};
class RTSPSrcProp : public Property
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("location", this->location);
      ar & make_nvp("user_id", this->user_id);
      ar & make_nvp("user_pw", this->user_pw);
    }
    virtual void setGstObjectProperty(GstElement *element)
    {
      assert(element != NULL);
      if(!(this->location.empty())){
        g_object_set(G_OBJECT(element), this->n_location, this->location.c_str(), 
            this->n_user_id, this->user_id.c_str(), this->n_user_pwd, 
            this->user_pw.c_str(), NULL);
      }
    }
    static const char *n_location;
    static const char *n_user_id;
    static const char *n_user_pwd;
    std::string location;
    std::string user_id;
    std::string user_pw;
};
class CameraSrcProp : public Property
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("fps_range", this->fpsRange);
    }
    virtual void setGstObjectProperty(GstElement *element)
    {
      assert(element != NULL);
      g_object_set(G_OBJECT(element), this->n_fpsRange, this->fpsRange.c_str(),
          NULL); 
    }
    static const char *n_fpsRange;
    std::string fpsRange;
};
class FileSinkProp : public Property
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("location", this->location);
    }
    virtual void setGstObjectProperty(GstElement *element)
    {
      assert(element != NULL);
      g_object_set(G_OBJECT(element), this->n_location, this->location.c_str(), NULL); 
    }
    static const char *n_location;
    std::string location;
};
class ConvProp : public Property
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("flip_method", this->flip_method);
    }
    virtual void setGstObjectProperty(GstElement *element)
    {
      assert(element != NULL);
      g_object_set(G_OBJECT(element), this->n_flip_method, this->flip_method, NULL); 
    }
    static const char *n_flip_method;
    unsigned flip_method;
};
class UDPSinkProp : public Property
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("host_ip", this->host);
    }
    virtual void setGstObjectProperty(GstElement *element)
    {
      assert(element != NULL);
      if(!(this->host.empty())){
      g_object_set(G_OBJECT(element), this->n_host, this->host.c_str(),
          this->n_port, this->port, NULL); 
      }
        //Do Nothing
    }
    static const char *n_host;
    static const char *n_port;
    std::string host;
    unsigned port;
};

Property* serializationSubElement(unsigned _sub_type);
Property* deSerializationSubElement(unsigned _sub_type);

class ElementProperty{
  public:
    ElementProperty();

    ElementProperty(elementType _type);
    ElementProperty(elementType _type, unsigned sub_type, 
        const char *_element_name, const char *_element_nickname);
    ~ElementProperty();

    void setElementName(std::string _element_name);
    std::string getElementName(void) const;

    void setElementNickName(std::string _element_nickname);
    std::string getElementNickName(void) const; 

    void setElementType(elementType _type);
    elementType getElementType(void) const;

    void setFps(unsigned _fps);
    unsigned getFps(void) const;

    void setWidth(unsigned _width);
    unsigned getWidth(void) const;

    void setHeight(unsigned _height);
    unsigned getHeight(void) const;
    
    unsigned getSubType(void) const { return this->sub_type; }
    void setSubType(unsigned _sub_type) { this->sub_type = _sub_type; } 

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version) 
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("element_name", this->element_name);
      ar & make_nvp("element_nickname", this->element_nickname);
      ar & make_nvp("element_type", this->type);
      ar & make_nvp("element_sub_type", this->sub_type); 
      switch(this->type)
      {
        case kSRC:
          ar & make_nvp("element_property", *camProp);
          ar & make_nvp("element_property", *rtspProp);
          ar & make_nvp("fps", this->fps);
          ar & make_nvp("width", this->width);
          ar & make_nvp("height", this->height);
          break;
        case kTEE:
          break;
        case kQUEUE:
          break;
        case kCONV:
          ar & make_nvp("element_property", *conProp); 
          break;
        case kENC:
          ar & make_nvp("element_property", *encProp);
          break;
        case kMUX:
          break;
        case kSINK:
          ar & make_nvp("element_property", *fileProp);
          ar & make_nvp("element_property", *udpProp);
          break;
        default:
          break;
      }
    }
    UDPSinkProp *udpProp;
    EncorderProp *encProp;
    RTSPSrcProp *rtspProp;
    CameraSrcProp *camProp;
    FileSinkProp *fileProp;
    ConvProp *conProp;
  protected:
    std::string element_name;
    std::string element_nickname;
    elementType type;
    unsigned sub_type;
    unsigned fps;     
    unsigned width;
    unsigned height;
};

class ElementXMLSerialization{
  private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar,
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("element_vector", (*this->_v_element_property));
    }
  public:
    void setVElementProperty(std::vector<ElementProperty*>
        *__v_element_property);
    std::vector<ElementProperty*>* getVElementProperty(void);
    ElementXMLSerialization() {}
    ElementXMLSerialization(std::vector<ElementProperty*> 
        *__v_element_property) : _v_element_property(__v_element_property) {}
  private:
    std::vector<ElementProperty*> *_v_element_property;
};

extern std::vector<ElementProperty*> *v_element_property;

void allocVectorElementProperty(void);
void setTx1DefaultProperty(void);
void printVectorElement(std::vector<ElementProperty*> *_v_element_property);
void printElement(ElementProperty *_element); 
void printProperty(ElementProperty *_element, elementType _type);
void deleteVectorElement(std::vector<ElementProperty*> *_v_element_property);
#endif /* OPEL_CAMERA_PROPERTY_H */
