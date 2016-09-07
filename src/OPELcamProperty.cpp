#include "OPELcamProperty.h"

void setTx1DefaultProperty(void)
{
   __OPEL_FUNCTION_ENTER__;
   std::vector<ElementProperty*> *element_vector = 
     new std::vector<ElementProperty*>(); 
   ElementProperty *nv_src = new ElementProperty(kSRC, "nvcamerasrc", "nvcamerasrc");
   ElementProperty *rtsp_src = new ElementProperty(kSRC, "rtspsrc", "rtspsrc");
   ElementProperty *tee = new ElementProperty(kTEE, "tee", "tee");
   ElementProperty *queue = new ElementProperty(kQUEUE, "queue", "queue");
   ElementProperty *i420_conv = new ElementProperty(kCONV, "nvvidconv", "nvvidconv");
   ElementProperty *h264_enc = new ElementProperty(kENC, "omxh264enc", "omxh264enc");
   ElementProperty *jpeg_enc = new ElementProperty(kENC, "nvjpegenc", "nvjpegenc");
   ElementProperty *mp4_mux = new ElementProperty(kMUX, "mp4mux", "mp4mux");
   ElementProperty *sink = new ElementProperty(kSINK, "filesink", "filesink");
   ElementProperty *jpeg_sink = new ElementProperty(kSINK, "filesink", "filesink"); 
   ElementProperty *udp_sink = new ElementProperty(kSINK, "udpsink", "udpsink");

   nv_src->camProp.fpsRange = charToString("30.0 30.0");
   h264_enc->encProp.bitrate = 8000000;
   h264_enc->encProp.quality_level = 0;
   i420_conv->conProp.flip_method = 2;
   sink->fileProp.location = charToString("/home/ubuntu/hihihi.mp4");
   jpeg_sink->fileProp.location = charToString("/home/ubuntu/hihihi.jpeg");

   element_vector->push_back(nv_src);
   element_vector->push_back(rtsp_src);
   element_vector->push_back(tee);
   element_vector->push_back(queue);
   element_vector->push_back(i420_conv);
   element_vector->push_back(h264_enc);
   element_vector->push_back(jpeg_enc);
   element_vector->push_back(mp4_mux);
   element_vector->push_back(sink);
   element_vector->push_back(jpeg_sink);
   element_vector->push_back(udp_sink);
  

   v_element_property = element_vector;

   __OPEL_FUNCTION_EXIT__;
}

void printProperty(ElementProperty *_element, elementType _type)
{
   switch(_type)
   {
      case kSRC:
          std::cout << "FPS Range : " << _element->camProp.fpsRange << std::endl;   
        break;
      case kTEE: 
        break;
      case kQUEUE: 
        break;
      case kCONV:
          std::cout << "Flip Method : " << _element->conProp.flip_method << std::endl;   
        break;
      case kENC:
          std::cout << "Quality Level : " << _element->encProp.quality_level << std::endl;   
          std::cout << "Bitrates : " << _element->encProp.bitrate << std::endl;   
        break;
      case kMUX:
        break;
      case kSINK:
          std::cout << "Location : " << _element->fileProp.location << std::endl;   
        break;
      default:
        break;
   }
}

void printElement(ElementProperty *_element) 
{
   assert(_element != NULL);
   elementType type_num = _element->getElementType();
   switch(type_num)
   {
      case kSRC:
        std::cout << "****SRC****" << std::endl; 
        break;
      case kTEE: 
        std::cout << "****TEE****" << std::endl;
        break;
      case kQUEUE:
        std::cout << "****QUEUE****" << std::endl;
        break;
      case kCONV:
        std::cout << "****CONV****" << std::endl;
        break;
      case kENC:
        std::cout << "****ENCORDER****" << std::endl;
        break;
      case kMUX:
        std::cout << "****MUXER****" << std::endl;
        break;
      case kSINK:
        std::cout << "****SINK****" << std::endl;
        break;
      default:
        break;
   }
   std::cout << "Element Name : " << _element->getElementName() << std::endl;
   std::cout << "Element NickName : " << _element->getElementNickName() << std::endl;
   printProperty(_element, type_num);
}

void printVectorElement(std::vector<ElementProperty*> *_v_element_property)
{
  assert(_v_element_property != NULL);
  std::vector<ElementProperty*> *v_tmp = v_element_property;
  std::vector<ElementProperty*>::iterator iter = (*v_tmp).begin();
  ElementProperty *tmp_element = NULL;
  
  for(iter; iter != (*v_tmp).end(); ++iter)
  {
    tmp_element = *iter;
    if(tmp_element != NULL)
       printElement(tmp_element); 
  }

}
void allocVectorElementProperty(void)
{
  assert(v_element_property == NULL);
  v_element_property = new std::vector<ElementProperty*>();
}

void deleteVectorElement(std::vector<ElementProperty*> *_v_element_property)
{ 
  assert(_v_element_property != NULL);
  std::vector<ElementProperty*> *v_tmp = v_element_property;
  std::vector<ElementProperty*>::iterator iter = (*v_tmp).begin();
  ElementProperty *tmp_element = NULL;
  
  for(iter; iter != (*v_tmp).end(); ++iter)
  {
    tmp_element = *iter;
    if(tmp_element != NULL){
      delete tmp_element;
      tmp_element = NULL;
    }
  }

}

void ElementXMLSerialization::setVElementProperty(
    std::vector<ElementProperty*> *__v_element_property)
{
  this->_v_element_property = __v_element_property;
}

std::vector<ElementProperty*>* ElementXMLSerialization::getVElementProperty(void)
{
  return this->_v_element_property;
}

ElementProperty::ElementProperty(elementType _type, const char *_element_name, 
   const char *_element_nickname)
{
  this->type = _type;
  std::string tmp_element_name = _element_name;
  std::string tmp_element_nickname = _element_nickname;

  this->element_name = tmp_element_name;
  this->element_nickname = tmp_element_nickname;
}
ElementProperty::ElementProperty(elementType _type)
{
  this->type = _type;
}

ElementProperty::~ElementProperty()
{ 
}

void ElementProperty::setElementName(std::string _element_name)
{
  this->element_name = _element_name;
}
std::string ElementProperty::getElementName(void) const
{
  return this->element_name;
}

void ElementProperty::setElementNickName(std::string _element_nickname)
{
  this->element_nickname = _element_nickname;
}
std::string ElementProperty::getElementNickName(void) const
{
  return this->element_nickname;
}

void ElementProperty::setElementType(elementType _type)
{
  this->type = _type;
} 
elementType ElementProperty::getElementType(void) const
{
  return this->type;
}
void ElementProperty::setFps(unsigned _fps)
{
  this->fps = _fps;
}
unsigned ElementProperty::getFps(void) const
{
  return this->fps;
}

void ElementProperty::setWidth(unsigned _width)
{
  this->width = _width;
}
unsigned ElementProperty::getWidth(void) const
{
  return this->width;
}

void ElementProperty::setHeight(unsigned _height)
{
  this->height = _height;
}
unsigned ElementProperty::getHeight(void) const
{
  return this->height;
}
