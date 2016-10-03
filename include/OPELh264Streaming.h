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
			

	private:
		static OPELH264Streaming *opel_h264_streaming;
		OPELH264Streaming();
		unsigned num_users;	

};


#endif /* OPEL_H264_STREAMING_H */
