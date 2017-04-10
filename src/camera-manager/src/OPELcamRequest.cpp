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
    OPEL_DBG_ERR("File Open Failed: %s", file_path);
    return ret;
  }
  ret = fwrite((char*)buffer, sizeof(char), size, fout);
  if(ret != size)
  {
    OPEL_DBG_ERR("File Write Failed: %s", file_path);
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
  //__OPEL_FUNCTION_EXIT__;
  return GST_FLOW_OK;
}

void checkRemainRequest(void)
{
  __OPEL_FUNCTION_ENTER__;
  bool ret;
  /*
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
  */

  for (unsigned camera_num=0; camera_num < OPEL_CAMERA_NUM; camera_num++) {
    OPELGstElementTx1 *tx1 = OPELGstElementTx1::getOPELGstElementTx1(camera_num);
    OPELGlobalVectorRequest *v_global_request = tx1->getGlobalVectorRequest();

    if(!v_global_request->isVectorEntryEmpty() ||
        tx1->getStreamingRequest()->getIsStreamingRun() ||
        tx1->getDelayRequest()->getIsStreamingRun()){
      OPEL_DBG_VERB("Request is not Empty Set playing true");
      tx1->setIsPlaying(true);
    }

    else{
      OPEL_DBG_VERB("Request is Empty Set is Playing false");
      tx1->setIsPlaying(false);
    }
  }

  __OPEL_FUNCTION_EXIT__;
}

bool openCVStart(DBusMessage *msg, OPELGstElementTx1 *tx1, 
    std::vector<typeElement*>*_type_element_vector)
{
  assert(tx1 != NULL && _type_element_vector != NULL);
  __OPEL_FUNCTION_ENTER__;
  bool ret = true;
  GstPad *tee_src_pad = NULL;
  GstPadTemplate *tee_src_pad_templ = NULL;
  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));
  OPELRawRequest *request_handle = OPELRawRequest::getInstance();
  //OPELGlobalVectorRequest *v_global_request =
  //    OPELGlobalVectorRequest::getInstance();
  OPELGlobalVectorRequest *v_global_request = tx1->getGlobalVectorRequest();
  request_handle->setTypeElementVector(_type_element_vector);

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_INVALID);

  OPEL_DBG_VERB("PID : %u", msg_handle->pid);

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
    OPEL_DBG_VERB("Is Not Playing");
    ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_READY);
    ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
    tx1->setIsPlaying(true);
  }
  else{
    OPEL_DBG_VERB("Is Already Playing");
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
  /*
  if(GST_EVENT_TYPE(GST_PAD_PROBE_INFO_DATA(info)) != GST_EVENT_EOS)
  {
  __OPEL_FUNCTION_EXIT__;
  return GST_PAD_PROBE_PASS;
  }
  */
  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  request_elements = (OPELRequestTx1*)user_data;

  pipeline = request_elements->getOPELGstElementTx1()->getPipeline();

  std::vector<typeElement*> *v_fly_type_elements =
      request_elements->getFlyTypeElementVector();

  //typeElement *tee = findByElementName(request_elements->getTypeElementVector(),
  //    "tee");
  typeElement *queue = findByElementName(request_elements->getFlyTypeElementVector(),
      "queue");

  if(!queue)
  {
    OPEL_DBG_ERR("Queue Element is NULL");
    return GST_PAD_PROBE_DROP;
  };

  OPEL_DBG_ERR("queue : %d", queue);
  for(int i=0; i<request_elements->getNumIter(); i++)
  {
    std::cout << "Removed Element : " << (*v_fly_type_elements)[i]->name->c_str()
        << std::endl;
    gst_element_set_state((*v_fly_type_elements)[i]->element, GST_STATE_NULL);
    gst_bin_remove(GST_BIN(pipeline), (*v_fly_type_elements)[i]->element);
  }

  gst_element_release_request_pad(request_elements->getSrcTee(), request_elements->getSrcPad());
  //sendReply(request_elements->getDBusConnection(),
  //    request_elements->getDBusMessage(), true);


  delete request_elements;

  checkRemainRequest();

  __OPEL_FUNCTION_EXIT__;
  return GST_PAD_PROBE_OK;
}

static gboolean timeOutCallback(gpointer _request_elements)
{
  assert(_request_elements != NULL);
  __OPEL_FUNCTION_ENTER__;
  OPEL_DBG_LOG("Time Out Callback Invoked");
  //OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();
  OPELRequestTx1 *request_elements = (OPELRequestTx1*)_request_elements; 
  OPELGstElementTx1 *tx1 = request_elements->getOPELGstElementTx1();
  OPELGlobalVectorRequest *v_global_request = tx1->getGlobalVectorRequest();
  typeElement *queue = findByElementName(request_elements->getFlyTypeElementVector(),
      "queue");

  OPEL_DBG_ERR("queue : %d", queue);

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

  OPEL_DBG_ERR("srcpad : %d", srcpad);

  gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
      event_probe_cb, (gpointer)_request_elements, NULL);

  OPEL_DBG_VERB("%s : Get Pad Element", queue->name->c_str());

  GstPad *queue_pad = gst_element_get_static_pad(queue->element, "sink");
  if(!queue_pad)
  {
    OPEL_DBG_ERR("Queue Pad is NULL");
    __OPEL_FUNCTION_EXIT__;
    return false;
  }
  // unlink First
  //GstPad *sinkpad = gst_element_get_static_pad(queue->element, "sink");
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

  //OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
  OPELGstElementTx1 *tx1 = request_handle->getOPELGstElementTx1();

  GstElement *pipeline = tx1->getPipeline();

  request_handle->defaultJpegElementFactory(request_handle->getMsgHandle()->file_path);
  _fly_type_element_v = request_handle->getFlyTypeElementVector();

  //typeElement *tee = findByElementName(_type_element_v, "tee");
  GstElement *raw_tee = tx1->getRawTee();
  typeElement *queue = findByElementName(_fly_type_element_v, "queue");

  //if(!tee || !queue)
  if(!raw_tee || !queue)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL"); 
    __OPEL_FUNCTION_EXIT__;
    return NULL;
  }

  //templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee->element), "src_%u");
  //src_pad = gst_element_request_pad(tee->element, templ, NULL, NULL);
  templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(raw_tee), "src_%u");
  src_pad = gst_element_request_pad(raw_tee, templ, NULL, NULL);
  
  request_handle->setSrcPad(src_pad);
  request_handle->setSrcTee(raw_tee);

  //OPEL_DBG_VERB("Obtained request pad %s for %s", gst_pad_get_name(templ),
  //    tee->name->c_str());

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

  OPELGstElementTx1 *tx1;
  GstPadTemplate *templ;
  std::string tmp_str;
  std::vector<typeElement*> *_fly_type_element_v;
  GstPad *src_pad; 

  request_handle->setTypeElementVector(_type_element_v);

  //OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();
  tx1 = request_handle->getOPELGstElementTx1();;
  GstElement *pipeline = tx1->getPipeline();

  gchar *name = gst_element_get_name(pipeline);
  OPEL_DBG_LOG(name);
  g_free(name);


  request_handle->defaultRecordingElementFactory(request_handle->getMsgHandle()->file_path);
  _fly_type_element_v = request_handle->getFlyTypeElementVector();

  //typeElement *tee = findByElementName(_type_element_v, "tee");
  GstElement *h264_tee = tx1->getH264Tee();
  typeElement *queue = findByElementName(_fly_type_element_v, "queue");

  //if(!tee || !queue)
  if(!h264_tee || !queue)
  {
    OPEL_DBG_ERR("Get TypeElement Pointer is NULL");
    __OPEL_FUNCTION_EXIT__;
    return NULL;
  }

  //templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(tee->element), "src_%u");
  //src_pad = gst_element_request_pad(tee->element, templ, NULL, NULL);
  templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(h264_tee), "src_%u");
  src_pad = gst_element_request_pad(h264_tee, templ, NULL, NULL);
  
  request_handle->setSrcPad(src_pad);
  request_handle->setSrcTee(h264_tee);

  OPEL_DBG_LOG("Obtained request pad %s for %s", gst_pad_get_name(templ),
      "tee");

  request_handle->defaultRecordingCapFactory();
  request_handle->defaultRecordingPipelineAdd(pipeline);


  OPEL_DBG_LOG(gst_element_get_name(h264_tee));
  OPEL_DBG_LOG(gst_element_get_name(queue->element));

  queue->pad = gst_element_get_static_pad(queue->element, "sink");
  
  OPEL_DBG_LOG("%" GST_PTR_FORMAT, gst_pad_get_allowed_caps(src_pad));
OPEL_DBG_LOG("%", GST_PTR_FORMAT, gst_pad_get_allowed_caps(queue->pad));

  
  GstPadLinkReturn ret;
  ret = gst_pad_link(request_handle->getSrcPad(), queue->pad);
  if (ret != GST_PAD_LINK_OK)
    OPEL_DBG_ERR("%d", ret);

  OPEL_DBG_LOG("%" GST_PTR_FORMAT, gst_pad_get_current_caps(request_handle->getSrcPad()));
  __OPEL_FUNCTION_EXIT__;
  return request_handle;
}

static DBusHandlerResult snapshotStart(DBusConnection *conn, DBusMessage *msg)
{
  OPEL_DBG_WARN("Get Snapshot Start Request");
  OPELGstElementTx1 *tx1;
  const char *file_path;
  int ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));
  OPELRequestTx1 *request_handle = new OPELRequestTx1();

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->camera_num),
      DBUS_TYPE_STRING, &file_path,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->width),
      DBUS_TYPE_UINT64, &(msg_handle->height),
      DBUS_TYPE_INVALID);

  msg_handle->file_path = file_path;

  OPEL_DBG_VERB("Camera Number : %u", msg_handle->camera_num);
  OPEL_DBG_VERB("File Path : %s", msg_handle->file_path);
  OPEL_DBG_VERB("PID : %u", msg_handle->pid);
  OPEL_DBG_VERB("Width : %u", msg_handle->width);
  OPEL_DBG_VERB("Height : %u", msg_handle->height);

  tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);
  OPELGlobalVectorRequest *v_global_request = tx1->getGlobalVectorRequest();

  request_handle->setMsgHandle(msg_handle);
  request_handle->setDBusConnection(conn);
  request_handle->setDBusMessage(msg);
  request_handle->setOPELGstElementTx1(tx1);

  request_handle = snapshotInit(tx1->getTypeElementVector(), request_handle);
  if(!request_handle)
  {
    OPEL_DBG_ERR("Recording Intialization Failed");
    __OPEL_FUNCTION_EXIT__;
    return DBUS_HANDLER_RESULT_HANDLED;
  }
  g_signal_connect(request_handle->getJpegAppSink()->element, "new-sample",
      G_CALLBACK(jpegBufferFromSinkCB), (gpointer)request_handle);
  v_global_request->pushRequest(request_handle);

  if(!(tx1->getIsPlaying()))
  {
    ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_READY);
    ret = gst_element_set_state(tx1->getPipeline(), GST_STATE_PLAYING);
    tx1->setIsPlaying(true);
  }
  else
    request_handle->defualtSnapshotGstSyncStateWithParent();

  //g_timeout_add_seconds(1, timeOutCallback, (void*)request_elements);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static gboolean sensorOverlay_cb(gpointer _tx1)
{
  OPELGstElementTx1 *tx1 = (OPELGstElementTx1*)_tx1;
  GstElement *text_overlay = tx1->getTOverlay();
  char text[64];

  // TODO: Need to Sensor name
  char* sensor_name ="BUTTON";
  char* sensorValue;
  char* valueType;
  char* valueName;

  DBusConnection* conn;
  DBusMessage* msg;
  DBusMessage* reply;
  DBusError error;

  if (tx1->getIsSensing() == false) {
    OPEL_DBG_LOG("Finish the Sensor Overlay");
    g_object_set(G_OBJECT(text_overlay), "text", "", NULL);
    return false;
  }

  msg = dbus_message_new_method_call("org.opel.sensorManager", // target for the method call
      "/", // object to call on
      "org.opel.sensorManager", // interface to call on
      "Get"); // method name
  if (NULL == msg) {
    OPEL_DBG_ERR("Message NULL (Fail to create message)");
    return false;
  }

  dbus_message_append_args(msg,
      DBUS_TYPE_STRING, &sensor_name,
      DBUS_TYPE_INVALID);

  // send message and get a reply
  dbus_error_init(&error);

  conn = tx1->getConn();
  if (conn == NULL){
    printf("Why null? \n");
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    return false;
  }

  reply = dbus_connection_send_with_reply_and_block(conn, msg, 500, &error); // Timeout 500 milli seconds
  // Blocked

  if (reply == NULL){
    OPEL_DBG_ERR("Get Null Reply");
    OPEL_DBG_ERR("Error : %s", error.message);
    return false;
  }
  dbus_error_free(&error);

  dbus_message_unref(msg);

  dbus_message_get_args(reply, NULL,
      DBUS_TYPE_STRING, &sensorValue,
      DBUS_TYPE_STRING, &valueType,
      DBUS_TYPE_STRING, &valueName,
      DBUS_TYPE_INVALID);

  dbus_message_unref(reply);
/*
  OPEL_DBG_VERB("Sensor Value : %s", sensorValue);
  OPEL_DBG_VERB("Value Type : %s", valueType);
  OPEL_DBG_VERB("Value Name : %s", valueName);
*/
  sprintf(text, "%s: %s (%s)", valueName, sensorValue, valueType);

  g_object_set(G_OBJECT(text_overlay), "text", text, NULL);

  return true;
}
static DBusHandlerResult recStop(DBusConnection *conn, DBusMessage *msg)
{
  OPEL_DBG_VERB("Get Recording Stop Request");
  OPELGstElementTx1 *tx1;
  unsigned request_pid;
  //dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64, &request_pid);
  // FIXME: Just for working (need to receive a camera number) => Maybe fixed? Must Check!!
  unsigned camera_num;
  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &camera_num,
      DBUS_TYPE_UINT64, &request_pid,
      DBUS_TYPE_INVALID);
  tx1 = OPELGstElementTx1::getOPELGstElementTx1(camera_num);
  OPELGlobalVectorRequest *v_global_request = tx1->getGlobalVectorRequest();

  OPELRequestTx1 *request_handle = v_global_request->getRequestByPid(request_pid);
  if(!request_handle)
  {
    OPEL_DBG_WARN("Request is not initialized yet (No PID Data)");
    __OPEL_FUNCTION_EXIT__;
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  dbusRequest *request = request_handle->getMsgHandle();
  if(!(request->is_start))
  {
    OPEL_DBG_WARN("Request is not started yet");
    // Just delete request_elements from global vector list
  }
  else
  {
    //Invoke Time out Callback
  }
  // get App pid
  // find pid vector and call the timeout callback
  // then delete the vector data
}

static DBusHandlerResult recStart(DBusConnection *conn, DBusMessage *msg)
{
  OPEL_DBG_WARN("Get Recording Start Request");
  const char *file_path;
  int ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));
  OPELRequestTx1 *request_handle = new OPELRequestTx1();
  OPELGstElementTx1 *tx1;
  //OPELGlobalVectorRequest *v_global_request = OPELGlobalVectorRequest::getInstance();

  dbus_message_get_args(msg, NULL,
    DBUS_TYPE_UINT64, &(msg_handle->camera_num),
    DBUS_TYPE_STRING, &file_path,
    DBUS_TYPE_UINT64, &(msg_handle->pid),
    DBUS_TYPE_UINT64, &(msg_handle->fps),
    DBUS_TYPE_UINT64, &(msg_handle->width),
    DBUS_TYPE_UINT64, &(msg_handle->height),
    DBUS_TYPE_UINT64, &(msg_handle->play_seconds),
    DBUS_TYPE_INVALID);

  msg_handle->file_path = file_path;

  OPEL_DBG_VERB("Camera Number : %u", msg_handle->camera_num);
  OPEL_DBG_VERB("File Path : %s", msg_handle->file_path);
  OPEL_DBG_VERB("PID : %u", msg_handle->pid);
  OPEL_DBG_VERB("FPS : %u", msg_handle->fps);
  OPEL_DBG_VERB("Width : %u", msg_handle->width);
  OPEL_DBG_VERB("Height : %u", msg_handle->height);
  OPEL_DBG_VERB("Playing Time : %usec", msg_handle->play_seconds);

  tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);

  request_handle->setMsgHandle(msg_handle);
  request_handle->setDBusConnection(conn);
  request_handle->setDBusMessage(msg);
  request_handle->setOPELGstElementTx1(tx1);
  //request_elements = recordingInit((std::vector<typeElement*>*)_type_element_vector,
  //    request_handle);
  request_handle = recordingInit(tx1->getTypeElementVector(), request_handle);

  if(!request_handle)
  {
    OPEL_DBG_ERR("Recording Initialization Failed");
    __OPEL_FUNCTION_EXIT__;
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  //input request_elements into global vector
  //v_global_request->pushRequest(request_elements);

  OPELGlobalVectorRequest *v_global_request = tx1->getGlobalVectorRequest();
  v_global_request->pushRequest(request_handle);

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
    request_handle->defaultRecordingGstSyncStateWithParent();
  }

  OPEL_DBG_LOG(gst_caps_to_string(gst_pad_get_allowed_caps(request_handle->getSrcPad())));
  //OPEL_DBG_LOG(gst_caps_to_string(gst_pad_query_caps(request_handle->getSrcPad(), NULL)));
  g_timeout_add_seconds(msg_handle->play_seconds, timeOutCallback, (void*)request_handle);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult sensorOverlayStart(DBusConnection *conn, DBusMessage *msg)
{
  OPELGstElementTx1 *tx1;
	const char* sensor_name;

  OPEL_DBG_LOG("Get Sensor Overlay Start Request");
	dbusSensorOverlayRequest *msg_handle = (dbusSensorOverlayRequest*)malloc(sizeof(dbusSensorOverlayRequest));

  dbus_message_get_args(msg, NULL,
    DBUS_TYPE_UINT64, &(msg_handle->camera_num),
    DBUS_TYPE_STRING, &sensor_name,
    DBUS_TYPE_UINT64, &(msg_handle->pid),
    DBUS_TYPE_INVALID);

	msg_handle->sensor_name = sensor_name;

  OPEL_DBG_VERB("Camera Number : %u", msg_handle->camera_num);
  OPEL_DBG_VERB("Sensor : %s", msg_handle->sensor_name);
  OPEL_DBG_VERB("PID : %u", msg_handle->pid);

  tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);
  tx1->setConn(conn);
  tx1->setIsSensing(true);
  g_timeout_add(100, sensorOverlay_cb, (void*)tx1); 

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult sensorOverlayStop(DBusConnection *conn, DBusMessage *msg)
{
  OPELGstElementTx1 *tx1;
	const char* sensor_name;

  OPEL_DBG_LOG("Get Sensor Overlay Stop Request");
	dbusSensorOverlayRequest *msg_handle = (dbusSensorOverlayRequest*)malloc(sizeof(dbusSensorOverlayRequest));

  dbus_message_get_args(msg, NULL,
    DBUS_TYPE_UINT64, &(msg_handle->camera_num),
    DBUS_TYPE_STRING, &sensor_name,
    DBUS_TYPE_UINT64, &(msg_handle->pid),
    DBUS_TYPE_INVALID);

	msg_handle->sensor_name = sensor_name;

  OPEL_DBG_VERB("Camera Number : %u", msg_handle->camera_num);
  OPEL_DBG_VERB("Sensor : %s", msg_handle->sensor_name);
  OPEL_DBG_VERB("PID : %u", msg_handle->pid);


  tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);
  tx1->setConn(conn);
  tx1->setIsSensing(false);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult delayStreamingStart(DBusConnection *conn, DBusMessage *msg)
{
	__OPEL_FUNCTION_ENTER__;
	OPELGstElementTx1 *tx1;

	OPEL_DBG_LOG("Get Delay Streaming Start Request");
	dbusInitDelayRequest *msg_handle = (dbusInitDelayRequest*)malloc(sizeof(dbusInitDelayRequest));

	dbus_message_get_args(msg, NULL,
			DBUS_TYPE_UINT64, &(msg_handle->camera_num),
			DBUS_TYPE_UINT64, &(msg_handle->delay),
			DBUS_TYPE_INVALID);

	OPEL_DBG_VERB("Camera Number : %u", msg_handle->camera_num);
	OPEL_DBG_VERB("Delay : %u", msg_handle->delay);

	tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);
	tx1->setDelay(msg_handle->delay);
	OPELH264Streaming *request_handle = tx1->getDelayRequest();

	dbusStreamingRequest *stream_msg_handle = (dbusStreamingRequest*)malloc(sizeof(dbusStreamingRequest));
	stream_msg_handle->camera_num = msg_handle->camera_num;
	stream_msg_handle->ip_address = "127.0.0.1";
	stream_msg_handle->port = msg_handle->camera_num + 3000;	// TODO
	stream_msg_handle->delay = msg_handle->delay;

	request_handle->setStreamingRequest(stream_msg_handle);
	request_handle->setOPELGstElementTx1(tx1);

  if(request_handle->getIsStreamingRun())
  {
    OPEL_DBG_VERB("Already Streaming Service is Running");
    return DBUS_HANDLER_RESULT_HANDLED;
  }
  else
  {
    OPEL_DBG_VERB("Start Streaming Service");
    request_handle->setIsStreamingRun(true);
    streamingStart(tx1->getTypeElementVector(), request_handle);
  }
	
	__OPEL_FUNCTION_EXIT__;
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult delayStreamingStop(DBusConnection *conn, DBusMessage *msg)
{
	__OPEL_FUNCTION_ENTER__;
  OPEL_DBG_WARN("Get Delay Streaming Stop Request");
  OPELGstElementTx1 *tx1;
  dbusStreamingRequest *msg_handle = new dbusStreamingRequest;
  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->camera_num),
      DBUS_TYPE_INVALID);
  
  OPEL_DBG_VERB("Camera Number : %d", msg_handle->camera_num);
  
  tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);
  
  OPELH264Streaming *request_handle = tx1->getDelayRequest();
  request_handle->setStreamingRequest(msg_handle);
  //// HAYUN: TODO:
  request_handle->setOPELGstElementTx1(tx1);
  if(request_handle->getIsStreamingRun())
  {
    OPEL_DBG_VERB("Stop the Streaming Service");
    // unlink them
    request_handle->detachedStreaming();
    request_handle->setIsStreamingRun(false);
  }
  else
  {
    OPEL_DBG_VERB("Already Stop the Streaming Service");
    return DBUS_HANDLER_RESULT_HANDLED;
  }

	__OPEL_FUNCTION_EXIT__;
	return DBUS_HANDLER_RESULT_HANDLED;
}


static GstPadProbeReturn event_rec_probe_cb(GstPad *pad, GstPadProbeInfo *info,
    gpointer user_data)
{
  assert(user_data != NULL);
  __OPEL_FUNCTION_ENTER__;
  OPELEventRecRequest *event_rec_request = (OPELEventRecRequest*)user_data;
	std::vector<GstElement*> *element_vector = event_rec_request->getElementVector();
  GstElement *pipeline = event_rec_request->getPipeline();

  gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));

  gst_element_set_state(pipeline, GST_STATE_NULL);
  //gst_bin_remove_many(GST_BIN(pipeline), (*element_vector)[0], (*element_vector)[1],
  //    (*element_vector)[2], (*element_vector)[3], (*element_vector)[4], (*element_vector)[5], NULL);
  gst_object_unref(pipeline);
  delete element_vector;
  delete event_rec_request;

  __OPEL_FUNCTION_EXIT__;
  return GST_PAD_PROBE_OK;
}


gboolean eventRecRequestCallback(gpointer _event_rec_request)
{
  assert(_event_rec_request != NULL);
  __OPEL_FUNCTION_ENTER__;
	OPELEventRecRequest *event_rec_request = (OPELEventRecRequest*)_event_rec_request;
	std::vector<GstElement*> *element_vector = event_rec_request->getElementVector();
	GstPad *sinkpad = gst_element_get_static_pad((*element_vector)[1], "sink");
  GstElement *pipeline = event_rec_request->getPipeline();

	//gst_pad_add_probe(sinkpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
	//		event_rec_probe_cb, (void*)event_rec_request, NULL);

  gst_pad_send_event(sinkpad, gst_event_new_eos());
  gst_object_unref(sinkpad);

	__OPEL_FUNCTION_EXIT__;
  return false;
}


static DBusHandlerResult eventRecStart(DBusConnection *conn, DBusMessage *msg)
{
	__OPEL_FUNCTION_ENTER__;
	OPELGstElementTx1 *tx1;
	const char* file_path;

	OPEL_DBG_LOG("Get Recording Start with Delay Request");
	dbusEventRecRequest *msg_handle = (dbusEventRecRequest*)malloc(sizeof(dbusEventRecRequest));

	dbus_message_get_args(msg, NULL,
			DBUS_TYPE_UINT64, &(msg_handle->camera_num),
			DBUS_TYPE_STRING, &file_path,
			DBUS_TYPE_UINT64, &(msg_handle->play_seconds),
			DBUS_TYPE_INVALID);
	msg_handle->file_path = file_path;
	
	OPEL_DBG_VERB("Camera Number : %u", msg_handle->camera_num);
	OPEL_DBG_VERB("File Path : %s", msg_handle->file_path);
	OPEL_DBG_VERB("Playing Time : %usec", msg_handle->play_seconds);
	
	tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);

  OPELEventRecRequest *request_handle = new OPELEventRecRequest(msg_handle->camera_num+3000,
      msg_handle->file_path, msg_handle->play_seconds);
  
  request_handle->pipelineMake();

	gst_element_set_state(request_handle->getPipeline(), GST_STATE_READY);
	gst_element_set_state(request_handle->getPipeline(), GST_STATE_PLAYING);

	g_timeout_add_seconds(request_handle->getPlaySeconds(), eventRecRequestCallback,(void*)request_handle); 
  
	__OPEL_FUNCTION_EXIT__;
	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult eventRecStop(DBusConnection *conn, DBusMessage *msg)
{
	__OPEL_FUNCTION_ENTER__;

	__OPEL_FUNCTION_EXIT__;
	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *_type_element_vector)
{
  __OPEL_FUNCTION_ENTER__;

  bool ret;

  if(dbus_message_is_signal(msg, dbus_interface, rec_start_request))
    return recStart(conn, msg);
  else if(dbus_message_is_signal(msg, dbus_interface, rec_stop_request))
    return recStop(conn, msg);
  else if(dbus_message_is_signal(msg, dbus_interface, snap_start_request))
    return snapshotStart(conn, msg);
  else if(dbus_message_is_signal(msg, dbus_interface, opencv_start_request))
  {
    OPEL_DBG_WARN("Get OpenCV Start Request");

    bool ret;
    unsigned num;
    OPELRawRequest *request_handle = OPELRawRequest::getInstance();
    OPELGstElementTx1 *tx1 = OPELGstElementTx1::getOPELGstElementTx1(0);

    if((request_handle->getNumUsers()) != 0)
    {
      request_handle->increaseNumUsers();
      OPEL_DBG_VERB("Num Users : %d", request_handle->getNumUsers());
    }
    else
    {
      //ret = openCVStart(msg, tx1, (std::vector<typeElement*>*)_type_element_vector);
      ret = openCVStart(msg, tx1, tx1->getTypeElementVector());
      request_handle->increaseNumUsers();
    }
    return DBUS_HANDLER_RESULT_HANDLED;
  }
  else if(dbus_message_is_signal(msg, dbus_interface, opencv_stop_request))
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
  else if(dbus_message_is_signal(msg, dbus_interface, streaming_start_request))
  {
    OPEL_DBG_WARN("Get Streaming Start Request");
    OPELGstElementTx1 *tx1;
    dbusStreamingRequest *msg_handle = new dbusStreamingRequest;
    dbus_message_get_args(msg, NULL,
        DBUS_TYPE_UINT64, &(msg_handle->camera_num),
        DBUS_TYPE_STRING, &(msg_handle->ip_address),
        DBUS_TYPE_UINT64, &(msg_handle->port),
        DBUS_TYPE_INVALID);

    OPEL_DBG_VERB("Camera Number : %d", msg_handle->camera_num);
    OPEL_DBG_VERB("IP Address : %s", msg_handle->ip_address);
    OPEL_DBG_VERB("Port Number : %d", msg_handle->port);
    msg_handle->delay = 0;

    tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);

    ////OPELH264Streaming *request_handle = new OPELH264Streaming();
    //OPELH264Streaming *request_handle = OPELH264Streaming::getInstance();
		OPELH264Streaming *request_handle = tx1->getStreamingRequest();
    request_handle->setStreamingRequest(msg_handle);
    //// HAYUN: TODO:
    request_handle->setOPELGstElementTx1(tx1);
    if(request_handle->getIsStreamingRun())
    {
      OPEL_DBG_VERB("Already Streaming Service is Running");
      return DBUS_HANDLER_RESULT_HANDLED;
    }
    else
    {
      OPEL_DBG_VERB("Start Streaming Service");
      request_handle->setIsStreamingRun(true);
      //streamingStart((std::vector<typeElement*>*)_type_element_vector,
      //    request_handle);
      streamingStart(tx1->getTypeElementVector(), request_handle);
    }
  }
  else if(dbus_message_is_signal(msg, dbus_interface, streaming_stop_request))
  {
    OPEL_DBG_WARN("Get Streaming Stop Request");
    OPELGstElementTx1 *tx1;
    dbusStreamingRequest *msg_handle = new dbusStreamingRequest;
    dbus_message_get_args(msg, NULL,
        DBUS_TYPE_UINT64, &(msg_handle->camera_num),
        DBUS_TYPE_INVALID);

    OPEL_DBG_VERB("Camera Number : %d", msg_handle->camera_num);

    tx1 = OPELGstElementTx1::getOPELGstElementTx1(msg_handle->camera_num);

    //OPELH264Streaming *request_handle = OPELH264Streaming::getInstance();
		OPELH264Streaming *request_handle = tx1->getStreamingRequest();
    request_handle->setStreamingRequest(msg_handle);
    //// HAYUN: TODO:
    request_handle->setOPELGstElementTx1(tx1);
    if(request_handle->getIsStreamingRun())
    {
      OPEL_DBG_VERB("Stop the Streaming Service");
      // unlink them
      request_handle->detachedStreaming();
      request_handle->setIsStreamingRun(false);
    }
    else
    {
      OPEL_DBG_VERB("Already Stop the Streaming Service");
      return DBUS_HANDLER_RESULT_HANDLED;
    }
  }
  else if(dbus_message_is_signal(msg, dbus_interface, sensor_overlay_start_request))
    return sensorOverlayStart(conn, msg);
  else if(dbus_message_is_signal(msg, dbus_interface, sensor_overlay_stop_request))
    return sensorOverlayStop(conn, msg);
	else if(dbus_message_is_signal(msg, dbus_interface, delay_streaming_start_request))
		return delayStreamingStart(conn, msg);
	else if(dbus_message_is_signal(msg, dbus_interface, delay_streaming_stop_request))
		return delayStreamingStop(conn, msg);
	else if(dbus_message_is_signal(msg, dbus_interface, event_rec_start_request))
		return eventRecStart(conn, msg);
	else if(dbus_message_is_signal(msg, dbus_interface, event_rec_stop_request))
		return eventRecStop(conn, msg);

  __OPEL_FUNCTION_EXIT__;
  return DBUS_HANDLER_RESULT_HANDLED;
}
