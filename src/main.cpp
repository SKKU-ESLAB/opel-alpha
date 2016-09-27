#include "OPELgstElementTx1.h"
#include "OPELcamRequest.h"
#include "OPELglobalRequest.h"
#include "OPELrawRequest.h"
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
static gboolean
message_cb(GstBus *bus, GstMessage *message, gpointer user_data)
{
	GError *err;
	gchar *debug_info;

	switch(GST_MESSAGE_TYPE(message))
	{
		case GST_MESSAGE_ERROR:
			         gst_message_parse_error(message, &err, &debug_info);
								 OPEL_DBG_ERR("Error Received From Element %s: %s\n",
								 GST_OBJECT_NAME (message->src), err->message);
								 OPEL_DBG_ERR("Debugging information: %s\n", debug_info ? debug_info : "none"); 
								 g_clear_error(&err);
								 g_free(debug_info);
			break;
		case GST_MESSAGE_WARNING:
			break;
		case GST_MESSAGE_EOS:
			OPEL_DBG_VERB("End-Of-Stream reached. \n");
			break;
	}
	return true;
}

void signalHandler(int signo)
{
  bool ret;
	OPEL_DBG_WARN("Signal handler Invoked");
	OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
  GstElement *pipeline = tx1->getPipeline();
  ret = gst_element_set_state(pipeline, GST_STATE_NULL);
  if(ret == GST_STATE_CHANGE_FAILURE)
  {
    OPEL_DBG_ERR("Unable to set the pipeline to the null state. \n");
  }
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
  __OPEL_FUNCTION_ENTER__; 
  allocVectorElementProperty();
  ElementXMLSerialization *_element_property_serialization = 
    new ElementXMLSerialization(v_element_property); 
  
  boost::archive::xml_iarchive ia(_xml_file);
  ia >> boost::serialization::make_nvp("OPEL_TX1_Element_Property", 
      (*_element_property_serialization));
  _xml_file.close();
  __OPEL_FUNCTION_EXIT__;
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

  bool ret;
  GstBus *bus;
  DBusError dbus_error;
  DBusConnection *dbus_conn;
  GstElement *_pipeline;
  std::vector<typeElement*> *_type_element_vector = NULL;
  ElementXMLSerialization *tx1_element_property = NULL; 
	OPELGlobalVectorRequest *global_vector_request = NULL;

  tx1_element_property = openXMLconfig(path_configuration_Tx1);
  if(tx1_element_property == NULL)
    writeXMLconfig(path_configuration_Tx1);

#if OPEL_LOG_VERBOSE
  printVectorElement(v_element_property);
#endif

  gst_init(&argc, &argv);
  
  loop = g_main_loop_new(NULL, false);
  dbus_error_init(&dbus_error);
  dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);
  if(dbus_error_is_set(&dbus_error))
  {
    OPEL_DBG_ERR("Dbus Bus System Registration Failed");
    dbus_error_free(&dbus_error);
    __OPEL_FUNCTION_EXIT__;
    return -1;
  }
  OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
 
  tx1->setElementPropertyVector(v_element_property);

#if OPEL_LOG_VERBOSE
  printTypeElement(tx1->getTypeElementVector());
#endif

  ret = tx1->OPELGstElementFactory();
  if(!ret)
    goto exit;
  ret = tx1->OPELGstElementPropFactory();
  if(!ret)
    goto exit;
  ret = tx1->OPELGstElementCapFactory();
  if(!ret)
    goto exit;
  ret = tx1->OPELGstPipelineMake();
  if(!ret)
    goto exit;
 
  _type_element_vector = tx1->getTypeElementVector();
  _pipeline = tx1->getPipeline();

	openCVStaticPipelineMake(tx1, _type_element_vector);					 	
  if(dbus_error_is_set(&dbus_error))
  {
    OPEL_DBG_ERR("Error Connecting to the D-bus Daemon");
    dbus_error_free(&dbus_error);
    goto exit;
  }

  signal(SIGINT, signalHandler);

  bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
  gst_bus_add_signal_watch(bus);
  g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(message_cb), NULL);

  gst_object_unref(GST_OBJECT(bus));
	

  if(ret == GST_STATE_CHANGE_FAILURE)
  {
    OPEL_DBG_ERR("Unable to set the pipeline to the playing state. \n");
    goto exit;
  }

  dbus_bus_add_match(dbus_conn, "type='signal', interface='org.opel.camera.daemon'", NULL);
  dbus_connection_add_filter(dbus_conn, msg_dbus_filter, 
      (void*)_type_element_vector, NULL);
  dbus_connection_setup_with_g_main(dbus_conn, NULL);

	gst_element_set_state(_pipeline, GST_STATE_PLAYING);
 	tx1->setIsPlaying(true);
	g_main_loop_run(loop);

exit:
  if(tx1_element_property != NULL)
    delete tx1_element_property;
  deleteVectorElement(v_element_property);
  if(v_element_property != NULL)
    delete v_element_property;
	//buggy
	/*	if(tx1 != NULL)
    delete tx1;*/
	if(loop != NULL)
    g_main_loop_unref(loop);
  if(dbus_conn != NULL)
    dbus_connection_unref(dbus_conn);
  if(bus != NULL)
    gst_object_unref(bus);
	
		if((global_vector_request = OPELGlobalVectorRequest::getInstance()) != NULL)
		delete global_vector_request;

	__OPEL_FUNCTION_EXIT__;
  return 0;
}
