#include "OPELcommonUtil.h"
#include "OPELgstElementTx1.h"
#include "OPELglobalRequest.h"
static GstPadProbeReturn event_probe_cb(GstPad *pad, GstPadProbeInfo *info,
    gpointer user_data)
{
  assert(user_data != NULL);
  __OPEL_FUNCTION_ENTER__;
  OPELRequestTx1 *request_elements = NULL;
  GstElement *pipeline = NULL;

  if(GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS)
  { 
    __OPEL_FUNCTION_EXIT__;
    return GST_PAD_PROBE_PASS;
  }
  
  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
  
  pipeline = (OPELGstElementTx1::getInstance())->getPipeline();
  request_elements = (OPELRequestTx1*)user_data;

  std::vector<typeElement*> *v_fly_type_elements = 
    request_elements->getFlyTypeElementVector();

  typeElement *tee = findByElementName(request_elements->getTypeElementVector(), 
      "tee");
  if(!tee)
  {
    OPEL_DBG_ERR("Tee Element is NULL");
    __OPEL_FUNCTION_EXIT__;
    return GST_PAD_PROBE_DROP;
  }

  for(int i=0; i<OPEL_NUM_DEFAULT_RECORDING_ELE; i++)
  {
#if OPEL_LOG_VERBOSE
      std::cout << "Removed Element : " << (*v_fly_type_elements)[i]->name->c_str() 
        << std::endl;
#endif
      gst_element_set_state((*v_fly_type_elements)[i]->element, GST_STATE_NULL);
      gst_bin_remove(GST_BIN(pipeline), (*v_fly_type_elements)[i]->element);
  }
  
  gst_element_release_request_pad(tee->element, request_elements->getSrcPad());
	
	delete request_elements;

	__OPEL_FUNCTION_EXIT__;
  return GST_PAD_PROBE_DROP;
}

static gboolean timeOutCallback(gpointer _request_elements)
{
  assert(_request_elements != NULL);
  __OPEL_FUNCTION_ENTER__;
#if OPEL_LOG_VERBOSE
  OPEL_DBG_VERB("Time Out Callback Invoked");
#endif
  OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();
	OPELRequestTx1 *request_elements = (OPELRequestTx1*)_request_elements; 
  typeElement *queue = findByElementName(request_elements->getFlyTypeElementVector(), 
      "queue");
 
	if(!queue)
  {
    OPEL_DBG_ERR("Not queue element in fly element vector");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  GstPad *srcpad = request_elements->getSrcPad();

	if(!srcpad)
  {
    OPEL_DBG_ERR("src pad is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }

  //GstPad block Impl
  GstPadProbeType mask = GST_PAD_PROBE_TYPE_BLOCK;
  gst_pad_add_probe(srcpad, mask, 
      event_probe_cb, (gpointer)_request_elements, NULL); 
  gst_object_unref(srcpad); 


#if OPEL_LOG_VERBOSE 
  OPEL_DBG_VERB("%s : Get Pad Element", queue->name->c_str());
#endif 

  GstPad *queue_pad = gst_element_get_static_pad(queue->element, "sink");
  if(!queue_pad)
  {
    OPEL_DBG_ERR("Queue Pad is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }
  gst_pad_send_event(queue_pad, gst_event_new_eos());
  gst_object_unref(queue_pad);

	v_global_request->deleteRequest(request_elements);
  
	__OPEL_FUNCTION_EXIT__;
  //Call Only Once 
  return false;
}

static OPELRequestTx1 *recordingInit(std::vector<typeElement*> *_type_element_v, 
    OPELRequestTx1 *request_handle)
{
  assert(_type_element_v != NULL && request_handle != NULL);
  __OPEL_FUNCTION_ENTER__;

  GstPadTemplate *templ;
  std::string tmp_str;
  std::vector<typeElement*> *_fly_type_element_v;
  GstPad *src_pad; 

  request_handle->setTypeElementVector(_type_element_v);

  OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();

  GstElement *pipeline = tx1->getPipeline();
 
	request_handle->defaultRecordingElementFactory(request_handle->getMsgHandle()->file_path);
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
  src_pad = gst_element_request_pad(tee->element, templ, NULL, NULL);

  request_handle->setSrcPad(src_pad);

//#if OPEL_LOG_VERBOSE
  OPEL_DBG_VERB("Obtained request pad %s for %s", gst_pad_get_name(templ), 
      tee->name->c_str());  
////#endif

  request_handle->defaultRecordingCapFactory();
  request_handle->defaultRecordingPipelineAdd(pipeline);

  queue->pad = gst_element_get_static_pad(queue->element, "sink");
  gst_pad_link(request_handle->getSrcPad(), queue->pad);

  __OPEL_FUNCTION_EXIT__;
  return request_handle;
}

DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *_type_element_vector)
{
  __OPEL_FUNCTION_ENTER__; 

  bool ret;
  OPELRequestTx1 *request_elements; 
  
	OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();

  if(dbus_message_is_signal(msg, dbus_interface, rec_init_request))
  {
    OPEL_DBG_WARN("Get Recording Initialization Request");
    const char *file_path;
 
    dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));
    OPELRequestTx1 *request_handle = new OPELRequestTx1();
    OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();

    dbus_message_get_args(msg, NULL, 
				DBUS_TYPE_STRING, &file_path,  
				DBUS_TYPE_UINT64, &(msg_handle->pid), 
				DBUS_TYPE_UINT64, &(msg_handle->fps),
        DBUS_TYPE_UINT64, &(msg_handle->width), 
				DBUS_TYPE_UINT64, &(msg_handle->height), 
				DBUS_TYPE_UINT64, &(msg_handle->play_seconds), 
				DBUS_TYPE_INVALID);

    msg_handle->file_path = file_path;

//#if OPEL_LOG_VERBOSE
    std::cout << "File Path : " << msg_handle->file_path << std::endl;
    std::cout << "PID : " << msg_handle->pid << std::endl;
		std::cout << "FPS : " << msg_handle->fps << std::endl;
    std::cout << "Width : " << msg_handle->width << std::endl;
    std::cout << "Height : " << msg_handle->height << std::endl;
    std::cout << "Playing Time : " << msg_handle->play_seconds << "sec" << std::endl;
//#endif

    request_handle->setMsgHandle(msg_handle);

    request_elements = 
      recordingInit((std::vector<typeElement*>*)_type_element_vector, 
          request_handle);

    if(!request_elements)
    {
      OPEL_DBG_ERR("Recording Initialization Failed");
      __OPEL_FUNCTION_EXIT__;
      return DBUS_HANDLER_RESULT_HANDLED;
    }
    //input request_elements into global vector
    v_global_request->pushRequest(request_elements);

    return DBUS_HANDLER_RESULT_HANDLED;
  }

	if(dbus_message_is_signal(msg, dbus_interface, rec_start_request))
	{
		OPEL_DBG_WARN("Get Recording Start Request");	
		unsigned request_pid;
		OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();
		dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64, &request_pid);

		OPELRequestTx1 *request_elements = v_global_request->getRequestByPid(request_pid);		
   	
		if(!request_elements)
		{
			OPEL_DBG_WARN("Request Is Not Init Yet (No PID Data)");
			__OPEL_FUNCTION_EXIT__;
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		
		dbusRequest *request = request_elements->getMsgHandle();	
   	request->is_start = true;	
		//dbusRequest set ture 
		if(!tx1->getIsPlaying()){
			ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
			if(ret == GST_STATE_CHANGE_FAILURE)
			{
				OPEL_DBG_ERR("Unable to set the pipeline to the playing state. \n");
				__OPEL_FUNCTION_EXIT__;
				return DBUS_HANDLER_RESULT_HANDLED;
			}
			tx1->setIsPlaying(true);
		}
		else
		{
			request_elements->defaultRecordingGstSyncStateWithParent();
			
		//	ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PAUSED);
//			ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
		}
		
		g_timeout_add_seconds(request->play_seconds, timeOutCallback, (void*)request_elements);   

	}
  
  if(dbus_message_is_signal(msg, dbus_interface, rec_stop_request))
  {
    OPEL_DBG_VERB("Get Recording Stop Request");
		unsigned request_pid;
		OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();
		dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64, &request_pid);

		OPELRequestTx1 *request_elements = v_global_request->getRequestByPid(request_pid);		
		if(!request_elements)
		{
			OPEL_DBG_WARN("Request is not initialized yet (No PID Data)");
			__OPEL_FUNCTION_EXIT__;
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		
		dbusRequest *request = request_elements->getMsgHandle();	
		if(!(request->is_start))
		{
			OPEL_DBG_WARN("Request is not started yet");
			//Just delete request_elements from global vector list 
		}
		else
		{
			//Invoke Time out Callback 
		}
		//get App pid
    // find pid vector and call the timeout callback
    // then delete the vector data
  }
  
  __OPEL_FUNCTION_EXIT__;
  return DBUS_HANDLER_RESULT_HANDLED;
}

