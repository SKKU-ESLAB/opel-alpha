#include "OPELcamRequest.h"
#include "OPELgstElementTx1.h"

static gboolean timeOutCallback(gpointer _tee)
{
  OPEL_DBG_ERR("Time Out Callback Invoked");
  typeElement *tee = (typeElement*)_tee;
  OPEL_DBG_ERR("%s : Get Pad Element", tee->name->c_str());
  GstPad *queue_pad = gst_element_get_static_pad(tee->element, "sink");
  gst_pad_send_event(queue_pad, gst_event_new_eos());
 
  return true;
}

static typeElement *recordingInit(std::vector<typeElement*> *_type_element_v)
{
   assert(_type_element_v != NULL);
   __OPEL_FUNCTION_ENTER__;
   GstPadTemplate *templ;
   std::string tmp_str;
   std::vector<typeElement*> *_fly_type_element_v;
   OPELRequestTx1 *request_handle = new OPELRequestTx1(); 

   request_handle->setTypeElementVector(_type_element_v);

   OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();

   GstElement *pipeline = tx1->getPipeline();
  
   request_handle->defaultRecordingElementFactory(tmp_str);
   _fly_type_element_v = request_handle->getFlyTypeElementVector();
   
   typeElement *tee = findByElementName(_type_element_v, "tee");
   typeElement *queue = findByElementName(_fly_type_element_v, "queue");
   
   if(!tee || !queue)
   {
      OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
      __OPEL_FUNCTION_EXIT__;
      return NULL;
   }
    
   templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee->element), "src_%u");
   tee->pad = gst_element_request_pad(tee->element, templ, NULL, NULL);

#if OPEL_LOG_VERBOSE
    OPEL_DBG_VERB("Obtained request pad %s for %s", gst_pad_get_name(templ), 
        tee->element->name->c_str());  
#endif
    
     request_handle->defaultRecordingCapFactory();
     request_handle->defaultRecordingPipelineAdd(pipeline);
   
   queue->pad = gst_element_get_static_pad(queue->element, "sink");
   gst_pad_link(tee->pad, queue->pad);

   return queue;
    
   __OPEL_FUNCTION_EXIT__;
}

DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *_type_element_vector)
{
  __OPEL_FUNCTION_ENTER__; 

  bool ret;
  typeElement *tee;
  OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
  tee = recordingInit((std::vector<typeElement*>*) _type_element_vector);

  
  ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
  if(ret == GST_STATE_CHANGE_FAILURE)
  {
      OPEL_DBG_ERR("Unable to set the pipeline to the playing state. \n");
      __OPEL_FUNCTION_EXIT__;
      return DBUS_HANDLER_RESULT_HANDLED;
  }
  

  g_timeout_add_seconds(10, timeOutCallback, (void*)tee);   

  
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
    ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
    if(ret == GST_STATE_CHANGE_FAILURE)
    {
      OPEL_DBG_ERR("Unable to set the pipeline to the playing state. \n");
      __OPEL_FUNCTION_EXIT__;
      return DBUS_HANDLER_RESULT_HANDLED;
    }
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

