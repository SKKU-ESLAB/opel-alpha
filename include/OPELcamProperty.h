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
#include <gst/gst.h>
#include "OPELdbugLog.h"

inline std::string charToString(const char* _str)
{
  std::string tmp = _str;
  return tmp;
}

inline gchar* stringToGchar(std::string* _str)
{
  unsigned int _str_size = _str->size();
  unsigned char *_char_str = (unsigned char*)malloc(_str_size+1);
  strcpy((char*)_char_str, _str->c_str()); 
  return (gchar*)_char_str;
}

typedef enum _elementType{
   kSRC = 0,
   kTEE,
   kQUEUE,
   kCONV,
   kENC,
   kMUX,
   kSINK,
}elementType;


class EncorderProp 
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
    unsigned quality_level;
    unsigned bitrate;
};
class RTSPSrcProp
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
    std::string location;
    std::string user_id;
    std::string user_pw;
};
class CameraSrcProp
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("fps_range", this->fpsRange);
    }
    std::string fpsRange;
};
class FileSinkProp
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("location", this->location);
    }
    std::string location;
};
class ConvProp
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("flip_method", this->flip_method);
    }
    unsigned flip_method;
};
class UDPSinkProp
{
  public:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("host_ip", this->host);
    }
    std::string host;
    unsigned port;
};

class ElementProperty{
  public:
    ElementProperty() {};

    ElementProperty(elementType _type);
    ElementProperty(elementType _type, const char *_element_name, 
        const char *_element_nickname);
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

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &ar, 
        const unsigned int version)
    {
      using boost::serialization::make_nvp;
      ar & make_nvp("element_name", this->element_name);
      ar & make_nvp("element_nickname", this->element_nickname);
      ar & make_nvp("element_type", this->type);

      switch(this->type)
      {
        case kSRC:
          ar & make_nvp("element_property", this->camProp);
          ar & make_nvp("element_property", this->rtspProp);
          break;
        case kTEE:
          break;
        case kQUEUE:
          break;
        case kCONV:
          ar & make_nvp("element_property", this->conProp);
          break;
        case kENC:
          ar & make_nvp("element_property", this->encProp);
          break;
        case kMUX:
          break;
        case kSINK:
          ar & make_nvp("element_property", this->fileProp);
          ar & make_nvp("element_property", this->udpProp);
          break;
        default:
          break;
      }
    }

    UDPSinkProp udpProp;
    EncorderProp encProp;
    RTSPSrcProp rtspProp;
    CameraSrcProp camProp;
    FileSinkProp fileProp;
    ConvProp conProp;

  protected:
    std::string element_name;
    std::string element_nickname;
    elementType type;
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
