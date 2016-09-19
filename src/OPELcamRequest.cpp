#include "OPELcamRequest.h"
#include "OPELgstElementTx1.h"

static void recordingInit(std::vector<typeElement*> *_type_element_v)
{
   assert(_type_element_v != NULL);
   __OPEL_FUNCTION_ENTER__;
   GstPadTemplate *templ;
   
   OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();

   GstElement *pipeline = tx1->getPipeline();

   typeElement *tee = findByElementName(_type_element_v, "tee");
   typeElement *queue = findByElementName(_type_element_v, "queue");
   typeElement *enc = findByElementName(_type_element_v, "omxh264enc");
   typeElement *mux = findByElementName(_type_element_v, "mp4mux");
   typeElement *sink = findByElementNameNSubType(_type_element_v, "filesink", kREC_SINK);

   
   if(!tee || !queue || !enc || !mux || !sink)
   {
      OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
      __OPEL_FUNCTION_EXIT__;
      return;
   }
   templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee->element), "src_%u");
   tee->pad = gst_element_request_pad(tee->element, templ, NULL, NULL);

#if OPEL_LOG_VERBOSE
    OPEL_DBG_VERB("Obtained request pad %s for convert branch", gst_pad_get_name(templ));  
#endif
    
   tx1->OPELGstElementRecordingCapFactory();
   tx1->OPELGstElementRecordingPipelineMake();
   
   queue->pad = gst_element_get_static_pad(queue->element, "sink");
   gst_pad_link(tee->pad, queue->pad);
     
    
   __OPEL_FUNCTION_EXIT__;
}

DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *_type_element_vector)
{
  __OPEL_FUNCTION_ENTER__; 

  OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
  
  recordingInit((std::vector<typeElement*>*) _type_element_vector);
  gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
  
  if(dbus_message_is_signal(msg, dbus_interface, rec_init_request))
  {
    OPEL_DBG_VERB("Get Recording Initialization Request");
    recordingInit((std::vector<typeElement*>*) _type_element_vector);
    /*     dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64,
       &(request->msg_type), DBUS_TYPE_UINT64, &(request->u.init_type), 
       DBUS_TYPE_UINT64, &(request->fps), DBUS_TYPE_UINT64, &(request->width),
       DBUS_TYPE_UINT64, &(request->height), DBUS_TYPE_UINT64, 
       &(request->num_frames), DBUS_TYPE_INVALID); */

  }

  if(dbus_message_is_signal(msg, dbus_interface, rec_start_request))
  {
     OPEL_DBG_VERB("Get Recording Start Request");
    /*     dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64,
       &(request->msg_type), DBUS_TYPE_UINT64, &(request->u.dynamic_type), 
       DBUS_TYPE_UINT64, &(request->fps), DBUS_TYPE_UINT64, &(request->width),
       DBUS_TYPE_UINT64, &(request->height), DBUS_TYPE_UINT64, 
       &(request->num_frames), DBUS_TYPE_INVALID); */
  }
  
  if(dbus_message_is_signal(msg, dbus_interface, rec_stop_request))
  {
     OPEL_DBG_VERB("Get Recording Start Request");
  
  
  }
  
  __OPEL_FUNCTION_EXIT__;
  return DBUS_HANDLER_RESULT_HANDLED;
}

