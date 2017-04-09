#ifndef OPEL_GST_ELEMENT_TX1_H
#define OPEL_GST_ELEMENT_TX1_H

#include "OPELgstElement.h"

class OPELRequestTx1 : public OPELRequest
{ 
  public:
    virtual bool defaultRecordingElementFactory(const char* file_path);
    virtual bool defaultRecordingPipelineAdd(GstElement *pipeline);
    virtual bool defaultRecordingCapFactory(void);
    void defaultRecordingGstSyncStateWithParent(void);

    virtual bool defaultJpegElementFactory(const char* file_path);
    virtual bool defaultJpegElementPipelineAdd(GstElement *pipeline);
    virtual bool defaultJpegCapFactory(void);
    void defualtSnapshotGstSyncStateWithParent(void);

    typeElement* getJpegAppSink(void)
    { return this->jpeg_app_sink; }
    void setJpegAppSink(typeElement* _jpeg_app_sink)
    { this->jpeg_app_sink = _jpeg_app_sink; }
/*
    OPELGstElementTx1* getOPELGstElementTx1(void)
    { return this->opel_gst_element_tx1; }
    void setOPELGstElementTx1(OPELGstElementTx1 *_opel_gst_element_tx1)
    { this->opel_gst_element_tx1 = _opel_gst_element_tx1; }
*/
  private:
    typeElement* jpeg_app_sink;
    
    //OPELGstElementTx1 *opel_gst_element_tx1;
};

class OPELGlobalVectorRequest;
class OPELH264Streaming;

class OPELGstElementTx1 : public OPELGstElement
{
  public:
    ~OPELGstElementTx1();
    //static OPELGstElementTx1* getInstance(void);
    virtual bool OPELGstElementFactory(void);
    virtual bool OPELGstPipelineMake(void);
    virtual bool OPELGstElementCapFactory(void);
    virtual bool OPELGstElementPropFactory(void);
    virtual void setElementPropertyVector(std::vector<ElementProperty*>
        *__v_element_property);

    GstElement* getPipeline(void)
    { return this->pipeline; }
    void setPipeline(GstElement* _pipeline)
    { this->pipeline = _pipeline; }

    bool getIsPlaying(void) const
    { return this->is_playing; }
    void setIsPlaying(bool _is_playing)
    { this->is_playing = _is_playing; }

    typeElement* getMainTee(void) const
    { return this->main_tee; }
    void setMainTee(typeElement* _main_tee)
    { this->main_tee = _main_tee; }
    GstElement* getRawTee(void) const
    { return this->raw_tee; }
    void setRawTee(GstElement* _raw_tee)
    { this->raw_tee = _raw_tee; }
    GstElement* getH264Tee(void) const
    { return this->h264_tee; }
    void setH264Tee(GstElement* _h264_tee)
    { this->h264_tee = _h264_tee; }


    unsigned getCameraNum(void) const
    { return this->camera_num; }
    void setCameraNum(unsigned _camera_num)
    { this->camera_num = _camera_num; }
    
    static void InitInstance(void);
    /*{
      opel_gst_element_bin[0] = new OPELGstElementTx1();
      opel_gst_element_bin[1] = new OPELGstElementTx1();
    }*/

    ////GstElement* getMainBin()
    ////{ return this->main_bin; }

    OPELGlobalVectorRequest* getGlobalVectorRequest(void)
    { return v_global_request; }

    static OPELGstElementTx1* getOPELGstElementTx1(unsigned camera_num)
    {
      if (camera_num == 0 || camera_num == 1)
        return opel_gst_element_tx1[camera_num];
      else
        return NULL;
    }

    GstElement* getTOverlay()
    { return this->t_overlay; }
    void setTOverlay(GstElement *_t_overlay)
    { this->t_overlay = _t_overlay; }
    GstElement* getTConv()
    { return this->t_conv; }
    void setTConv(GstElement *_t_conv)
    { this->t_conv = _t_conv; }
    GstElement* getCOverlay()
    { return this->c_overlay; }
    void setCOverlay(GstElement *_c_overlay)
    { this->c_overlay = _c_overlay; }
    DBusConnection* getConn()
    { return this->conn; }
    void setConn(DBusConnection* _conn)
    { this->conn = _conn; }
    bool getIsSensing()
    { return this->is_sensing; }
    void setIsSensing(bool _is_sensing)
    { this->is_sensing = _is_sensing; }

		OPELH264Streaming* getStreamingRequest(void);
		OPELH264Streaming* getDelayRequest(void);
		unsigned getDelay()
		{ return this->delay; }
		void setDelay(unsigned _delay)
		{ this->delay = _delay; }

  private:
    //static OPELGstElementTx1 *opel_gst_element_tx1;
    bool is_playing;
    typeElement *main_tee;
    GstElement *raw_tee;
    GstElement *h264_tee;
    GstElement *raw_queue;
    OPELGstElementTx1();
    GstElement *pipeline;
    //GstElement *main_bin;
    GstCaps *src2conv;

    GstElement *t_overlay;	// textoverlay plugin
    GstElement *t_conv;
		GstElement *c_overlay;	// clockoverlay plugin
    DBusConnection *conn;
    bool is_sensing;

		OPELH264Streaming *streaming_request;
		OPELH264Streaming *delay_request;
		unsigned delay;
    
    static OPELGstElementTx1* opel_gst_element_tx1[2];
    unsigned camera_num;
    OPELGlobalVectorRequest *v_global_request;
};

class OPELEventRecRequest : public OPELRequest
{
	public:
		OPELEventRecRequest();
		OPELEventRecRequest(unsigned _port, const char *_file_path, unsigned _play_seconds) {
		  this->port = _port;
		  this->file_path = _file_path;
      this->play_seconds = _play_seconds;
    }
		~OPELEventRecRequest();
		bool pipelineMake();

    virtual bool defaultRecordingElementFactory(const char *file_path)
    {return false;}
    virtual bool defaultRecordingPipelineAdd(GstElement *pipeline)
    {return false;}
    virtual bool defaultRecordingCapFactory(void)
    {return false;}

    virtual bool defaultJpegElementFactory(const char* file_path)
    {return false;}
    virtual bool defaultJpegElementPipelineAdd(GstElement *pipeline)
    {return false;}
    virtual bool defaultJpegCapFactory(void)
    {return false;}

		GstElement *getPipeline(void)
		{ return this->pipeline; }
		std::vector<GstElement*> *getElementVector(void)
		{ return this->_element_vector; }
    unsigned getPlaySeconds()
    { return this->play_seconds; }


	private:
		GstElement *pipeline;
		std::vector<GstElement*> *_element_vector;
		unsigned port;
		const char *file_path;
		unsigned play_seconds;
};

bool gstElementFactory(std::vector<typeElement*>
    *_type_element_vector);
void gstElementPropFactory(std::vector<typeElement*>
    *_type_element_vector);
void freeTypeElementMember(typeElement *type_element);
void initializeTypeElement(typeElement *type_element,
    ElementProperty *element_property);
void printTypeElement(std::vector<typeElement*> *_type_element_vector);
void OPELgstSyncStateWithParent(std::vector<typeElement*> *_v_ele_property);

#endif /* OPEL_GST_ELEMENT_TX1_H */
