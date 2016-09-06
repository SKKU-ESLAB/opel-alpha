#include "OPELcamProperty.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

std::vector<ElementProperty*> *v_element_property;

const char *path_configuration_Tx1 = "/home/ubuntu/opel-alpha/OPELTx1Configuartion.xml";

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
  
  if(xml_file.good())
    element_property = readXMLconfig(xml_file); 
  
  __OPEL_FUNCTION_EXIT__;
  return element_property;
}

ElementXMLSerialization* readXMLconfig(std::ifstream& _xml_file)
{
  ElementXMLSerialization *_element_property = new ElementXMLSerialization();
  _element_property->setVElementProperty(v_element_property);    
  boost::archive::xml_iarchive ia(_xml_file);

  return _element_property;
}

void writeXMLconfig(const char *_path_xml)
{
  __OPEL_FUNCTION_ENTER__;
  ElementXMLSerialization _element_property_serialization;
  ElementProperty *tx1_element_set = NULL;

  //Default Tx1 Property Setup
  setTx1DefaultProperty();

  _element_property_serialization.setVElementProperty(v_element_property);

  std::ofstream xml_file;
  xml_file.open(_path_xml, std::ios::out);
  if(xml_file.good()){ 
    boost::archive::xml_oarchive oa(xml_file);
    oa << boost::serialization::make_nvp("OPEL_TX1_Element_Property",
        _element_property_serialization);
    //    oa << BOOST_SERIALIZATION_NVP(_element_property_serialization);
  } else {
    OPEL_DBG_VERB("Failed to Open XML File");
  }
}

int main(int argc, char** argv)
{
  __OPEL_FUNCTION_ENTER__; 
  int ret;
  ElementXMLSerialization *tx1_element_property = NULL; 

  tx1_element_property = openXMLconfig(path_configuration_Tx1);

  if(tx1_element_property == NULL)
    writeXMLconfig(path_configuration_Tx1);

  printVectorElement(v_element_property);

  

exit:
  if(tx1_element_property != NULL)
    delete tx1_element_property;
  deleteVectorElement(v_element_property);
  if(v_element_property != NULL)
    delete v_element_property;
  __OPEL_FUNCTION_EXIT__;
  return 0;
}
