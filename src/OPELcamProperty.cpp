#include "OPELcamProperty.h"

template<class Archive> void ElementXMLSerialization::serialize(Archive & ar,
    const unsigned int version)
{
     ar & this->v_element_property; 
}

void ElementXMLSerialization::setVElementProperty(
    const std::vector<ElementProperty*> &_v_element_property)
{
  this->v_element_property = _v_element_property;
}

std::vector<ElementProperty*> ElementXMLSerialization::getVElementProperty(void)
{
  return this->v_element_property;
}

ElementProperty::ElementProperty(elementType _type)
{
  assert(_type > 0 && _type < MAX_ELEMENT_TYPE_NUM);
 
  this->type = _type;
  this->encProp = NULL;
  this->rtspProp = NULL;
  this->camProp = NULL;
  this->conProp = NULL;
  this->fileProp = NULL;
  this->udpProp = NULL;

  switch(_type)
  {
    case kPIPELINE:
      break;
    case kSRC:
        this->camProp = (cameraSrcProp*)malloc(sizeof(cameraSrcProp));
        this->rtspProp = (rtspSrcProp*)malloc(sizeof(rtspSrcProp));
      break;
    case kTEE:
      break;
    case kQUEUE:
      break;
    case kCONV:
        this->conProp = (convProp*)malloc(sizeof(convProp));
      break;
    case kENC:
        this->encProp = (encorderProp*)malloc(sizeof(encorderProp));
      break;
    case kMUX:
      break;
    case kSINK:
        this->fileProp = (fileSinkProp*)malloc(sizeof(fileSinkProp));
        this->udpProp = (udpSinkProp*)malloc(sizeof(udpSinkProp));
      break;
    default:
      break;
  }
}

ElementProperty::~ElementProperty()
{ 
  if(this->encProp != NULL)
    free(this->encProp);
  if(this->rtspProp != NULL)
    free(this->rtspProp);
  if(this->camProp != NULL)
    free(this->camProp);
  if(this->conProp != NULL)
    free(this->conProp);
  if(this->fileProp != NULL)
    free(this->fileProp);
  if(this->udpProp != NULL)
    free(this->udpProp);
}

void ElementProperty::setElementName(std::string _element_name)
{
  this->element_name = _element_name;
}
std::string ElementProperty::getElementName(void)
{
  return this->element_name;
}

void ElementProperty::setElementNickName(std::string _element_nickname)
{
  this->element_nickname = _element_nickname;
}
std::string ElementProperty::getElementNickName(void)
{
  return this->element_nickname;
}

void ElementProperty::setElementType(elementType _type)
{
  this->type = _type;
} 
elementType ElementProperty::getElementType(void)
{
  return this->type;
}
void ElementProperty::setFps(unsigned _fps)
{
  this->fps = _fps;
}
unsigned ElementProperty::getFps(void)
{
  return this->fps;
}

void ElementProperty::setWidth(unsigned _width)
{
  this->width = _width;
}
unsigned ElementProperty::getWidth(void)
{
  return this->width;
}

void ElementProperty::setHeight(unsigned _height)
{
  this->height = _height;
}
unsigned ElementProperty::getHeight(void)
{
  return this->height;
}
