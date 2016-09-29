#include "OPELrawRequest.h"
#include "OPELgstElementTx1.h"
#include <gst/app/gstappsink.h>

const char *sem_path = "ORG.OPEL.CAMERA";

OPELRawRequest *OPELRawRequest::opel_raw_request = NULL;

bool OPELRawRequest::initializeSemAndSharedMemory(void)
{
	__OPEL_FUNCTION_ENTER__;
	int ret = initSharedMemorySpace(1, RAW_DEFAULT_BUF_SIZE, 
			&(this->shm_ptr), SHM_KEY_FOR_BUFFER);
	if(this->shm_ptr == NULL)
	{
		OPEL_DBG_ERR("Shared memory space has NULL address");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	if(ret == -1){
		OPEL_DBG_ERR("Shared memory space initialization failed");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	this->shm_id = ret;

	if(initSemaphore(sem_path, &(this->sem)) == false)
	{
		OPEL_DBG_ERR("Semaphore initailization failed");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	__OPEL_FUNCTION_EXIT__;
	return true;
}

OPELRawRequest *OPELRawRequest::getInstance(void)
{
	if(opel_raw_request == NULL)
		opel_raw_request = new OPELRawRequest();
	return opel_raw_request;
}

OPELRawRequest::OPELRawRequest()
{
	this->app_sink = (typeElement*)malloc(sizeof(typeElement));
	this->_v_fly_type_element = 
		new std::vector<typeElement*>(OPEL_NUM_DEFAULT_RAW_ELE);
	this->num_users = 0;
}

OPELRawRequest::~OPELRawRequest()
{
	if(this->app_sink != NULL)
	  free(this->app_sink);
	
//	if(this->_v_fly_type_element != NULL)
//		delete this->_v_fly_type_element;
  
	uinitSharedMemorySpace(this->shm_id);
	uinitSemaphore(sem_path, this->sem);
}

static void initAppSinkElement(typeElement* app_sink)
{
	assert(app_sink != NULL);
	const char* n_app_sink = "appsink";
	std::string *str = new std::string(n_app_sink);
	app_sink->name = str;
	app_sink->type = kAPP_SINK;
	
}
static void initAppSinkElementProp(typeElement* app_sink)
{
	__OPEL_FUNCTION_ENTER__;
	assert(app_sink != NULL);
	g_object_set(G_OBJECT(app_sink->element), "emit-signals", TRUE, 
			"drop", TRUE, "max-buffers", (guint)1, "wait-on-eos", TRUE, NULL);

	//	g_object_set(G_OBJECT(app_sink->element));
	__OPEL_FUNCTION_EXIT__;
}

bool OPELRawRequest::defaultOpenCVElementFactory()
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	
	this->num_iter = OPEL_NUM_DEFAULT_RAW_ELE;

	typeElement *_conv = findByElementNameNSubType(this->_v_type_element,
			      "nvvidconv", kBGR);
	typeElement *_queue = findByElementName(this->_v_type_element,
			      "queue");
	if(!_conv || !this->app_sink || !_queue)
	{
		OPEL_DBG_ERR("conv and app_sink is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
 	
	typeElement *_new_conv = (typeElement*)malloc(sizeof(typeElement));
	_new_conv->element_prop = _conv->element_prop;
	initializeTypeElement(_new_conv, _new_conv->element_prop);

	typeElement *_new_queue = (typeElement*)malloc(sizeof(typeElement));
	_new_queue->element_prop = _queue->element_prop;
	initializeTypeElement(_new_queue, _new_queue->element_prop);
	
	initAppSinkElement(this->app_sink);	
	
	(*this->_v_fly_type_element)[0] = _new_queue;
	(*this->_v_fly_type_element)[1] = _new_conv;
	(*this->_v_fly_type_element)[2] = this->app_sink;
  
	gstElementFactory(this->_v_fly_type_element);
	
	_new_conv->prop->setGstObjectProperty(_new_conv->element);
	initAppSinkElementProp(this->app_sink);
	
	__OPEL_FUNCTION_EXIT__;
	return true;
}

bool OPELRawRequest::defaultOpenCVCapFactory()
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
 	__OPEL_FUNCTION_ENTER__;
	char caps_buffer[256];

	gint width = (guint)RAW_480P_WIDTH;
	gint height = (guint)RAW_480P_HEIGHT;

	typeElement *_conv = findByElementNameNSubType(this->_v_fly_type_element,
			      "nvvidconv", kBGR);
	
	if(!_conv)
	{
		OPEL_DBG_ERR("conv element is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	sprintf(caps_buffer, "video/x-raw, format=(string){BGRx}, \
			width=(int){%d}, height=(int){%d}", width, height);
	
	_conv->caps = gst_caps_from_string(caps_buffer);	

	if(!_conv->caps)
	{
		OPEL_DBG_ERR("Conv Caps is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}
	__OPEL_FUNCTION_EXIT__;
	return true;
}

bool OPELRawRequest::defaultOpenCVElementPipelineAdd(GstElement *pipeline)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL
			&& pipeline != NULL);

 	__OPEL_FUNCTION_ENTER__;
	bool ret = true;
	typeElement *_queue = findByElementName(this->_v_fly_type_element,
			      "queue");
	typeElement *_conv = findByElementNameNSubType(this->_v_fly_type_element,
			      "nvvidconv", kBGR);
	typeElement *_app_sink = findByElementName(this->_v_fly_type_element,
			      "appsink");

	if(!_conv || !_app_sink || !_queue)
	{
		OPEL_DBG_ERR("conv app_sink is NULL");
		__OPEL_FUNCTION_EXIT__;
		return false;
	}

	gst_bin_add_many(GST_BIN(pipeline), _queue->element, _conv->element, 
			_app_sink->element, NULL);
	
	ret = gst_element_link(_queue->element, _conv->element);
	if(!ret)
	{
		OPEL_DBG_ERR("Gst element link failed");
		__OPEL_FUNCTION_EXIT__; 
		return ret;
	}
	
	ret = gst_element_link_filtered(_conv->element, _app_sink->element, _conv->caps);
	if(!ret)
	{
		OPEL_DBG_ERR("Gst element link filtered failed");
		__OPEL_FUNCTION_EXIT__; 
		return ret;
	}


	__OPEL_FUNCTION_EXIT__;
	return ret;
}

void OPELRawRequest::defaultOpenCVPadLink(GstPad *main_tee_src_pad)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL
			&& main_tee_src_pad != NULL);
	__OPEL_FUNCTION_ENTER__;
	typeElement *_queue = findByElementName(this->_v_fly_type_element,
			      "queue");
	if(!_queue)
	{
		OPEL_DBG_ERR("queue is NULL"); 
		__OPEL_FUNCTION_ENTER__;
		return;
	}
	_queue->pad = gst_element_get_static_pad(_queue->element, "sink");
	gst_pad_link(main_tee_src_pad, _queue->pad);
__OPEL_FUNCTION_EXIT__;
}

void OPELRawRequest::defaultOpenCVGstSyncStateWithParent(void)
{
	assert(this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	OPELgstSyncStateWithParent(this->_v_fly_type_element);
	__OPEL_FUNCTION_EXIT__;
}

static int uinitSharedMemorySpace(int _shm_id)
{
	if(-1 == shmctl(_shm_id, IPC_RMID, 0))
	{
		OPEL_DBG_ERR("Failed to remove shared memory space");
		return 0;
	}
	return 1;
}

static int initSharedMemorySpace(int _req_count, int _buffer_size, 
		void** _shm_ptr, key_t _shmkey)
{
	int shmid;
	shmid = shmget((key_t)_shmkey, 
			(_buffer_size*_req_count)+(sizeof(int)*_req_count), 0666|IPC_CREAT);
/*
	OPEL_DBG_VERB("shmkey : %d", _shmkey);
*/
	if(shmid == -1)
	{
		OPEL_DBG_ERR("shmget failed : ");
		return -1;
	}
	*_shm_ptr = shmat(shmid, NULL, 0);

	if(*_shm_ptr == (void*)-1)
	{
		OPEL_DBG_ERR("shmget failed : ");
		return -1;
	}
	return shmid;
}

static GstPadProbeReturn detachCB(GstPad *pad, GstPadProbeInfo *info,
		gpointer user_data)
{
	assert(user_data != NULL);
	__OPEL_FUNCTION_ENTER__;

	OPELRawRequest *request_handle = NULL;
	OPELGstElementTx1 *tx1 = OPELGstElementTx1::getInstance();

	gst_pad_remove_probe(pad, GST_PAD_PROBE_INFO_ID(info));
	request_handle = (OPELRawRequest*)user_data;	

	std::vector<typeElement*> *v_fly_type_elements = 
		request_handle->getFlyTypeElementVector();
	
	for(int i=0; i< request_handle->getNumIter(); i++)
	{
		std::cout << "Removed Elements : " << (*v_fly_type_elements)[i]->name->c_str()
			<< std::endl;
		gst_element_set_state((*v_fly_type_elements)[i]->element, GST_STATE_NULL);
		gst_bin_remove(GST_BIN(tx1->getPipeline()), (*v_fly_type_elements)[i]->element); 
	}
	gst_element_release_request_pad(tx1->getMainTee()->element, 
			request_handle->getGstMainTeePad());

	__OPEL_FUNCTION_EXIT__;
	return GST_PAD_PROBE_OK;
}

bool OPELRawRequest::detachedOpenCVPipeline(void)
{
	assert(this->_v_type_element != NULL && this->_v_fly_type_element != NULL);
	__OPEL_FUNCTION_ENTER__;
	GstPad *sink_pad;
	typeElement *_queue = findByElementName(this->_v_fly_type_element,
			      "queue");
	
	if(!_queue || !this->main_tee_pad)
	{
		OPEL_DBG_ERR("queue and main_tee_pad is NULL");
		__OPEL_FUNCTION_EXIT__; 
		return false;
	}
	
	gst_pad_add_probe(this->main_tee_pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
			detachCB, (gpointer)this, NULL);

	sink_pad = gst_element_get_static_pad(_queue->element, "sink");
	gst_pad_unlink(this->main_tee_pad, sink_pad);
	
	gst_pad_send_event(sink_pad, gst_event_new_eos());
	gst_object_unref(sink_pad);

	__OPEL_FUNCTION_EXIT__;
	return true;
}

static bool initSemaphore(const char *path, sem_t **_sem)
{
	assert(path != NULL);
	*_sem = sem_open(path, O_CREAT, 0666, 1);
	if((*_sem) == SEM_FAILED)
	{
		OPEL_DBG_ERR("Semaphore Open Failed");
		sem_unlink(path);
		return false;
	}
	return true;		
}

static void uinitSemaphore(const char *path, sem_t *sem)
{
	sem_close(sem);
	sem_unlink(path);
}














