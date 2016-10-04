#include "OPELcommonUtil.h"
#include "OPELgstElementTx1.h"
#include "OPELglobalRequest.h"
#include "OPELrawRequest.h"
#include "OPELh264Streaming.h"
#include <gst/app/gstappsink.h>
#include <errno.h>
void closeFile(FILE *_fout)
{
	if(_fout)
		fclose(_fout);
}

unsigned fileWrite(const char *file_path, char *buffer, unsigned size)
{
	FILE *fout;
	int ret=0;
	fout = fopen(file_path, "w+");
	if(!fout){
		OPEL_DBG_ERR("File Open Failed");
		return ret;
	}
  ret = fwrite((char*)buffer, sizeof(char), size, fout);	
	if(ret != size)
	{
		OPEL_DBG_ERR("File Write Failed ");	
		closeFile(fout);
		return 0;
	}
	closeFile(fout);
	return ret;
}

GstFlowReturn jpegBufferFromSinkCB(GstElement *elt, gpointer data)
{
	__OPEL_FUNCTION_ENTER__;
	GstSample *sample; 
	GstBuffer *buffer;
	GstMapInfo map;
	OPELRequestTx1 *request_handle = (OPELRequestTx1*)data;
	sample = gst_app_sink_pull_sample(GST_APP_SINK(elt));
	buffer = gst_sample_get_buffer(sample);	
	if(gst_buffer_map(buffer, &map, GST_MAP_READ)){
		if(map.size != fileWrite(request_handle->getMsgHandle()->file_path, 
					(char*)map.data, (unsigned)map.size))
		{
			 OPEL_DBG_ERR("Fail to Write");
			 return GST_FLOW_OK;	//give it a shot
		}
	}
	timeOutCallback(request_handle);	
	__OPEL_FUNCTION_EXIT__;
	return GST_FLOW_EOS;
}

GstFlowReturn bufferFromSinkCB(GstElement *elt, gpointer data)
{
	//__OPEL_FUNCTION_ENTER__;
	OPELRawRequest *for_shared = OPELRawRequest::getInstance();
	GstSample *sample;
	GstBuffer *buffer;
	GstMapInfo map;
	sem_t* sem = for_shared->getSemaphore();
	sample = gst_app_sink_pull_sample(GST_APP_SINK(elt));
	buffer = gst_sample_get_buffer(sample);
	char* buffer_ptr = (char*)for_shared->getBufferPtr();
	unsigned int* buffer_size_ptr = 
		(unsigned int*)for_shared->getBufferSizePtr();
	
	if(gst_buffer_map(buffer, &map, GST_MAP_READ)){
			sem_wait(sem);
			*buffer_size_ptr = map.size;
			memcpy((char*)buffer_ptr, (char*)map.data, map.size);
			gst_buffer_unmap(buffer, &map);
			sem_post(sem);
	}
	gst_buffer_unref(buffer);
//	__OPEL_FUNCTION_EXIT__;
	return GST_FLOW_OK;
}

void checkRemainRequest(void)
{
	__OPEL_FUNCTION_ENTER__;
	bool ret;
	OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();
	OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
	if(v_global_request->isVectorEntryEmpty()){
		OPEL_DBG_VERB("Request is Empty Set is Playing false");
		tx1->setIsPlaying(false);
	}
	else{		
		OPEL_DBG_VERB("Request is not Empty Set playing true"); 
		tx1->setIsPlaying(true);	
	}
	__OPEL_FUNCTION_EXIT__;
}

bool openCVStart(DBusMessage *msg, OPELGstElementTx1 *tx1, 
		std::vector<typeElement*>*_type_element_vector)
{
	assert(tx1 != NULL && _type_element_vector != NULL);
	__OPEL_FUNCTION_ENTER__;
	bool ret;
	GstPad *tee_src_pad = NULL;
	GstPadTemplate *tee_src_pad_templ = NULL;

	
	ret = true;
	dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));
	OPELRawRequest *request_handle = OPELRawRequest::getInstance();
	OPELGlobalVectorRequest *v_global_request = 
		OPELGlobalVectorRequest::getInstance();
	request_handle->setTypeElementVector(_type_element_vector);

	dbus_message_get_args(msg, NULL,
			DBUS_TYPE_UINT64, &(msg_handle->pid),
			DBUS_TYPE_INVALID);

#if OPEL_LOG_VERBOSE
	std::cout << "PID : " << msg_handle->pid << std::endl;
#endif

	request_handle->setMsgHandle(msg_handle);
	request_handle->defaultOpenCVElementFactory();
	request_handle->defaultOpenCVCapFactory();
	request_handle->defaultOpenCVElementPipelineAdd(tx1->getPipeline());
	request_handle->initializeSemAndSharedMemory();

	tee_src_pad_templ = gst_element_class_get_pad_template(
			GST_ELEMENT_GET_CLASS(tx1->getMainTee()->element), "src_%u");
	tee_src_pad = gst_element_request_pad(tx1->getMainTee()->element, 
			tee_src_pad_templ, NULL, NULL);
	
	request_handle->setGstMainTeePad(tee_src_pad);
	request_handle->defaultOpenCVPadLink(tee_src_pad);		

	g_signal_connect (request_handle->getAppSink()->element, "new-sample", 
			G_CALLBACK(bufferFromSinkCB), NULL);
	
	if(!(tx1->getIsPlaying()))
	{
		OPEL_DBG_ERR("Is Not Playing");
		ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_READY);	
		ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING); 
		tx1->setIsPlaying(true);
	}
	else{
		OPEL_DBG_ERR("Is Already Playing");
		request_handle->defaultOpenCVGstSyncStateWithParent();
	}

	__OPEL_FUNCTION_EXIT__;
	return ret;
}

static GstPadProbeReturn event_probe_cb(GstPad *pad, GstPadProbeInfo *info,
    gpointer user_data)
{
  assert(user_data != NULL);
	__OPEL_FUNCTION_ENTER__;
	OPELRequestTx1 *request_elements = NULL;
  GstElement *pipeline = NULL;
/*	if(GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS)
	{
		__OPEL_FUNCTION_EXIT__;
		return GST_PAD_PROBE_PASS;
	}*/
	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
		
  pipeline = (OPELGstElementTx1::getInstance())->getPipeline();
  request_elements = (OPELRequestTx1*)user_data;

  std::vector<typeElement*> *v_fly_type_elements = 
    request_elements->getFlyTypeElementVector();

  typeElement *tee = findByElementName(request_elements->getTypeElementVector(), 
      "tee");
  typeElement *queue = findByElementName(request_elements->getFlyTypeElementVector(), 
      "queue");
  
	if(!tee || !queue)
  {
    OPEL_DBG_ERR("Tee & Queue Element is NULL");
    return GST_PAD_PROBE_DROP;
  };

#if OPEL_LOG_VERBOSE
	OPEL_DBG_ERR("queue : %d", queue);
#endif 
	for(int i=0; i<request_elements->getNumIter(); i++)
	{
#if OPEL_LOG_VERBOSE
		std::cout << "Removed Element : " << (*v_fly_type_elements)[i]->name->c_str() 
			<< std::endl;
#endif
		gst_element_set_state((*v_fly_type_elements)[i]->element, GST_STATE_NULL);
		gst_bin_remove(GST_BIN(pipeline), (*v_fly_type_elements)[i]->element);
	}
  
	gst_element_release_request_pad(tee->element, request_elements->getSrcPad());
//	sendReply(request_elements->getDBusConnection(), 
	//		request_elements->getDBusMessage(), true);


	delete request_elements;
	
	checkRemainRequest();	

	__OPEL_FUNCTION_EXIT__;
  return GST_PAD_PROBE_OK;
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

#if OPEL_LOG_VERBOSE
	OPEL_DBG_ERR("queue : %d", queue);
#endif 

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

#if OPEL_LOG_VERBOSE
	OPEL_DBG_ERR("srcpad : %d", srcpad);
#endif

	gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, 
      event_probe_cb, (gpointer)_request_elements, NULL); 

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
//unlink First
//	GstPad *sinkpad = gst_element_get_static_pad(queue->element, "sink");
	gst_pad_unlink(request_elements->getSrcPad(), queue_pad);	
  
	gst_pad_send_event(queue_pad, gst_event_new_eos());
  gst_object_unref(queue_pad);
	
	v_global_request->deleteRequest(request_elements);
 
	__OPEL_FUNCTION_EXIT__;
  return false;
}

static OPELRequestTx1 *snapshotInit(std::vector<typeElement*> *_type_element_v, 
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
	 
	request_handle->defaultJpegElementFactory(request_handle->getMsgHandle()->file_path);
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
//#endif

	request_handle->defaultJpegCapFactory();
  request_handle->defaultJpegElementPipelineAdd(pipeline);

  queue->pad = gst_element_get_static_pad(queue->element, "sink");
  gst_pad_link(request_handle->getSrcPad(), queue->pad);

  __OPEL_FUNCTION_EXIT__;
  return request_handle;
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

#if OPEL_LOG_VERBOSE
  OPEL_DBG_VERB("Obtained request pad %s for %s", gst_pad_get_name(templ), 
      tee->name->c_str());  
#endif

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
  OPELRequestTx1 *request_elements = NULL; 
  
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
		request_handle->setDBusConnection(conn);
		request_handle->setDBusMessage(msg);
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

		msg_handle->is_start = true;	
		if(!(tx1->getIsPlaying()))
		{
			OPEL_DBG_WARN("Get Recording not started");	
			ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_READY);
			ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
		  tx1->setIsPlaying(true);	
		}
		else{
			OPEL_DBG_WARN("Get Recording already Started");	
			request_elements->defaultRecordingGstSyncStateWithParent();
		}
		
		g_timeout_add_seconds(msg_handle->play_seconds, timeOutCallback, (void*)request_elements);   
		
    return DBUS_HANDLER_RESULT_HANDLED;
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

	if(dbus_message_is_signal(msg, dbus_interface, snap_start_request))
	{
		OPEL_DBG_WARN("Get Snapshot Start Request");
		const char *file_path;

		dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));
		OPELRequestTx1 *request_handle = new OPELRequestTx1(); 
		OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();

		dbus_message_get_args(msg, NULL, 
				DBUS_TYPE_STRING, &file_path,  
				DBUS_TYPE_UINT64, &(msg_handle->pid), 
				DBUS_TYPE_UINT64, &(msg_handle->width), 
				DBUS_TYPE_UINT64, &(msg_handle->height), 
				DBUS_TYPE_INVALID);

		msg_handle->file_path = file_path;

#if OPEL_LOG_VERBOSE
		std::cout << "File Path : " << msg_handle->file_path << std::endl;
		std::cout << "PID : " << msg_handle->pid << std::endl;
		std::cout << "Width : " << msg_handle->width << std::endl;
		std::cout << "Height : " << msg_handle->height << std::endl;
#endif

		request_handle->setMsgHandle(msg_handle);
		request_handle->setDBusConnection(conn);
		request_handle->setDBusMessage(msg);
		
		request_elements = snapshotInit((std::vector<typeElement*>*)_type_element_vector, 
				request_handle);
		if(!request_elements)
		{
			OPEL_DBG_ERR("Recording Intialization Failed");
			__OPEL_FUNCTION_EXIT__;
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		g_signal_connect(request_handle->getJpegAppSink()->element, "new-sample",
				G_CALLBACK(jpegBufferFromSinkCB), (gpointer)request_handle);
		v_global_request->pushRequest(request_elements);

		if(!(tx1->getIsPlaying()))
		{
				ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_READY);
				ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
				tx1->setIsPlaying(true);
		}
		else
			 request_handle->defualtSnapshotGstSyncStateWithParent();		
			
//		g_timeout_add_seconds(1, timeOutCallback, (void*)request_elements);   
	}
	
	if(dbus_message_is_signal(msg, dbus_interface, opencv_start_request))
	{
		OPEL_DBG_WARN("Get OpenCV Start Request");

		bool ret;
		unsigned num;
		OPELRawRequest *request_handle = OPELRawRequest::getInstance();

		if((request_handle->getNumUsers()) != 0)
		{
			request_handle->increaseNumUsers();	
			OPEL_DBG_VERB("Num Users : %d", request_handle->getNumUsers());
		}
		else
		{
   		ret = openCVStart(msg, tx1, (std::vector<typeElement*>*)_type_element_vector);
			request_handle->increaseNumUsers();	
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	
	if(dbus_message_is_signal(msg, dbus_interface, opencv_stop_request))
	{
		OPEL_DBG_WARN("Get OpenCV Stop Request");
		unsigned cur_attached_user;
		OPELRawRequest *request_handle = OPELRawRequest::getInstance();
		request_handle->decreaseNumUsers();
		if((cur_attached_user=request_handle->getNumUsers()) == 0)
		{
			OPEL_DBG_VERB("No Attached Users starting to termination OpenCV Service");
			//Termination Logic	
		 	if(!(request_handle->detachedOpenCVPipeline())) 
				return DBUS_HANDLER_RESULT_HANDLED;
		}
		else
			OPEL_DBG_VERB("Num Users : %d", cur_attached_user);
	}

	if(dbus_message_is_signal(msg, dbus_interface, streaming_start_request))
	{
		OPEL_DBG_WARN("Get Streaming Start Request");
		dbusStreamingRequest *msg_handle = new dbusStreamingRequest;
		dbus_message_get_args(msg, NULL, 
				DBUS_TYPE_STRING, &(msg_handle->ip_address), 
				DBUS_TYPE_UINT64, &(msg_handle->port), 
				DBUS_TYPE_INVALID);

//#if OPEL_LOG_VERBOSE
		OPEL_DBG_VERB("IP Address : %s", msg_handle->ip_address);
		OPEL_DBG_VERB("Port Number : %d", msg_handle->port);
//#endif

		OPELH264Streaming *request_handle = OPELH264Streaming::getInstance();
		request_handle->setStreamingRequest(msg_handle);
		if(request_handle->getIsStreamingRun())
		{
			OPEL_DBG_VERB("Already Streaming Service is Running");
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		else
		{
			OPEL_DBG_VERB("Start Streaming Service");
			request_handle->setIsStreamingRun(true);
			streamingStart((std::vector<typeElement*>*)_type_element_vector,
					request_handle);
		}
	}
	
	if(dbus_message_is_signal(msg, dbus_interface, streaming_stop_request))
	{
		OPEL_DBG_WARN("Get Streaming Stop Request");
		OPELH264Streaming *request_handle = OPELH264Streaming::getInstance();
		if(request_handle->getIsStreamingRun())
		{
			OPEL_DBG_VERB("Stop the Streaming Service");
			//unlink them 		   	
			request_handle->detachedStreaming();
			request_handle->setIsStreamingRun(false);
		}
		else
		{
			OPEL_DBG_VERB("Already Stop the Streaming Service");
  		return DBUS_HANDLER_RESULT_HANDLED;
		}
	}
	__OPEL_FUNCTION_EXIT__;
  return DBUS_HANDLER_RESULT_HANDLED;
}