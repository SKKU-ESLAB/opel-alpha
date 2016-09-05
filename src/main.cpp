#include "OPELcamProperty.h"
#include "OPELdbugLog.h"

const char *path_configuration_Tx1 = "~/opel-alpha/ \
OPELTx1Configuartion.xml";

ElementXMLSerialization* readXMLconfig(std::ifstream& _xml_file);
ElementXMLSerialization* openXMLconfig(const char *_path_xml);

ElementXMLSerialization* openXMLconfig(const char *_path_xml)
{
  __OPEL_FUNCTION_ENTER__;
  ElementXMLSerialization *element_property;
  std::ifstream xml_file;
  xml_file.open(_path_xml, std::ios::in); 
  if(xml_file.fail())
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
ElementXMLSerialization* readXMLconfig(std::ifstream& _xml_file)
{
  ElementXMLSerialization *_element_property = new ElementXMLSerialization();
  _element_property->setVElementProperty(v_element_property);    
  boost::archive::xml_iarchive ia(_xml_file, 0); 
  return _element_property;
}
void writeXMLconfig(const char *_path_xml)
{
  


}
int main(int argc, char** argv)
{
  __OPEL_FUNCTION_ENTER__; 
  int ret;
  ElementXMLSerialization *tx1_element_property = NULL; 

  tx1_element_property = openXMLconfig(path_configuration_Tx1);

  if(tx1_element_property == NULL)
    writeXMLconfig(path_configuration_Tx1);



exit:
  if(tx1_element_property != NULL)
    delete tx1_element_property;
  __OPEL_FUNCTION_EXIT__;
  return 0;
}
