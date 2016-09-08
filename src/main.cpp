#include "OPELgstElementTx1.h"
#include "OPELcamRequest.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <signal.h>
#include <unistd.h>

const char *path_configuration_Tx1 = "/home/ubuntu/opel-alpha/OPELTx1Configuartion.xml";

static GMainLoop *loop;

std::vector<ElementProperty*> *v_element_property;

void signalHandler(int signo);

ElementXMLSerialization* readXMLconfig(std::ifstream& _xml_file);
ElementXMLSerialization* openXMLconfig(const char *_path_xml);


void writeXMLconfig(const char *_path_xml);

void signalHandler(int signo)
{
  if(loop && g_main_loop_is_running(loop))
    g_main_loop_quit(loop);
}

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
  assert(!_xml_file.fail());
  
  allocVectorElementProperty();
  ElementXMLSerialization *_element_property_serialization = 
    new ElementXMLSerialization(v_element_property); 
  
  boost::archive::xml_iarchive ia(_xml_file);
  ia >> boost::serialization::make_nvp("OPEL_TX1_Element_Property", 
      (*_element_property_serialization));
  _xml_file.close();
  return _element_property_serialization;
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
  } else {
    OPEL_DBG_VERB("Failed to Open XML File");
  }
  xml_file.close();
}

int main(int argc, char** argv)
{
  __OPEL_FUNCTION_ENTER__; 
  int ret;
  DBusError dbus_error;
  DBusConnection *dbus_conn;

  ElementXMLSerialization *tx1_element_property = NULL; 

  tx1_element_property = openXMLconfig(path_configuration_Tx1);
  if(tx1_element_property == NULL)
    writeXMLconfig(path_configuration_Tx1);

  //  printVectorElement(v_element_property);
  gst_init(&argc, &argv);
  
  loop = g_main_loop_new(NULL, false);
  dbus_error_init(&dbus_error);
  dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);

  OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
  
  tx1->setElementPropertyVector(v_element_property);

  //  printTypeElement(tx1->getTypeElementVector());

  tx1->OPELGstElementFactory();
  
  if(dbus_error_is_set(&dbus_error))
  {
    OPEL_DBG_ERR("Error Connecting to the D-bus Daemon");
    dbus_error_free(&dbus_error);
    goto exit;
  }

  dbus_bus_add_match(dbus_conn, "type='signal',interface='org.opel.camera.daemon'", NULL);
  dbus_connection_add_filter(dbus_conn, msg_dbus_filter, loop, NULL);
  dbus_connection_setup_with_g_main(dbus_conn, NULL);
  signal(SIGINT, signalHandler);
  
  

  
   
  g_main_loop_run(loop);



exit:
  if(tx1_element_property != NULL)
    delete tx1_element_property;
  deleteVectorElement(v_element_property);
  if(v_element_property != NULL)
    delete v_element_property;
  if(tx1 != NULL)
    delete tx1;
  if(loop != NULL)
    g_main_loop_unref(loop);
  if(dbus_conn != NULL)
    dbus_connection_unref(dbus_conn);
  __OPEL_FUNCTION_EXIT__;
  return 0;
}
