#ifndef OPEL_CAMERA_PROPERTY_H
#define OPEL_CAMERA_PROPERTY_H
#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <fstream>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#define MAX_ELEMENT_TYPE_NUM 8


typedef enum _elementType{
   kPIPELINE = 0,
   kSRC,
   kTEE,
   kQUEUE,
   kCONV,
   kENC,
   kMUX,
   kSINK,
}elementType;

typedef struct _encorderProp{
  unsigned quality_level;
  unsigned bitrate; 
}encorderProp;

typedef struct _rtspSrcProp{
  std::string location;
  std::string user_id;
  std::string user_pw;
}rtspSrcProp;

typedef struct _cameraSrcProp{
  std::string fpsRange;
}cameraSrcProp;

typedef struct _fileSinkProp{
  std::string file_path;
}fileSinkProp;

typedef struct _convProp{
  unsigned flip_method;
}convProp;

typedef struct _udpSinkProp{
  std::string host;
  unsigned port;
}udpSinkProp;


class ElementProperty{
  public:
    ElementProperty(elementType _type);
    ~ElementProperty();

    void setElementName(std::string _element_name);
    std::string getElementName(void);
    
    void setElementNickName(std::string _element_nickname);
    std::string getElementNickName(void); 
    
    void setElementType(elementType _type);
    elementType getElementType(void);
    
    void setFps(unsigned _fps);
    unsigned getFps(void);

    void setWidth(unsigned _width);
    unsigned getWidth(void);

    void setHeight(unsigned _height);
    unsigned getHeight(void);
    
    udpSinkProp* udpProp;
    encorderProp* encProp;
    rtspSrcProp* rtspProp;
    cameraSrcProp* camProp;
    fileSinkProp* fileProp;
    convProp* conProp;
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
        const unsigned int version);
    std::vector<ElementProperty*> v_element_property;
  public:
    void setVElementProperty(const std::vector<ElementProperty*>
        &_v_element_property);
    std::vector<ElementProperty*> getVElementProperty(void);
};

static std::vector<ElementProperty*> v_element_property;

#endif /* OPEL_CAMERA_PROPERTY_H */
