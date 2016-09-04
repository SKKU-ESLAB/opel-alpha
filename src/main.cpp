#include "OPELcamProperty.h"
#include "OPELdbugLog.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

const char *path_configuration_Tx1 = "~/opel-alpha/ \
OPELTx1Configuartion.xml";

ElementXMLSerialization* readXMLconfig(std::ifstream &_xml_file);
ElementXMLSerialization* openXMLconfig(const char *_path_xml);

ElementXMLSerialization* openXMLconfig(const char *_path_xml)
{
  __OPEL_FUNCTION_ENTER__;
  ElementXMLSerialization *element_property;
  std::ifstream xml_file(_path_xml); 
  if(xml_file.bad())
  {
    OPEL_DBG_VERB("No Configuration XML File");
    xml_file.close();
  __OPEL_FUNCTION_EXIT__;
    return NULL;
  }
  element_property = readXMLconfig(xml_file); 
  __OPEL_FUNCTION_EXIT__;
  return element_property;
}
ElementXMLSerialization* readXMLconfig(std::ifstream &_xml_file)
{
  ElementXMLSerialization _element_property;
  _element_property.setVElementProperty(v_element_property);    
     
}

int main(int argc, char** argv)
{
  __OPEL_FUNCTION_ENTER__; 
  int ret;
  ElementXMLSerialization *tx1_element_property = NULL; 

  tx1_element_property = openXMLconfig(path_configuration_Tx1);
  if(tx1_element_property == NULL)
  {
    //make xml file
  }



exit:
  if(tx1_element_property != NULL)
    delete tx1_element_property;
  __OPEL_FUNCTION_EXIT__;
  return 0;
}
