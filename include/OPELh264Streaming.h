#ifndef OPEL_H264_STREAMING_H
#define OPEL_H264_STREAMING_H
#include "OPELgstElement.h"
#include "OPELgstElementTx1.h"

class OPELH264Streaming : public OPELRequest
{
	public:
		static OPELH264Streaming* getInstance(void);
		~OPELH264Streaming();
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
		
		bool defaultStreamingFactory(void);
		bool defaultStreamingCapFactory(void);
		bool defaultStreamingPipelineAdd(GstElement *pipeline);
		void defaultStreamingPadLink(GstPad *tee_src_pad);
		void defaultStreamingSyncStateWithParent(void);

		void setIsStreamingRun(bool _is_streaming_run)
		{  this->is_streaming_run = _is_streaming_run; }
		bool getIsStreamingRun(void)
		{  return this->is_streaming_run; }
	private:
		static OPELH264Streaming *opel_h264_streaming;
		OPELH264Streaming();
		bool is_streaming_run;

};

bool streamingStart(std::vector<typeElement*> *_type_element_v,
		    OPELH264Streaming *request_handle);

#endif /* OPEL_H264_STREAMING_H */
