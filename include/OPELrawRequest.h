#ifndef OPEL_RAW_REQUEST_H
#define OPEL_RAW_REQUEST_H
#include "OPELgstElement.h"
#include "OPELgstElementTx1.h"
class OPELRawRequest : public OPELRequest
{
	public:
	  static OPELRawRequest* getInstance(void);
		~OPELRawRequest();
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

		bool defaultOpenCVElementFactory();
		bool defaultOpenCVCapFactory();
		bool defaultOpenCVElementPipelineAdd(GstElement *pipeline);
		void defaultOpenCVPadLink(GstPad*);
		void defaultOpenCVGstSyncStateWithParent(void);

		unsigned getNumUsers(void) const
		{ return this->num_users; }
	
		void setNumUsers(unsigned _num_users)
		{	  this->num_users = _num_users;   } 

		void increaseNumUsers(void)
		{ this->num_users++; }
		bool getIsRun(void) const
		{ return this->is_run; }
		void setIsRun(bool _is_run) 
		{ this->is_run = _is_run; }

		typeElement* getAppSink(void)
		{  return this->app_sink; }
	private:
		static OPELRawRequest *opel_raw_request;
		OPELRawRequest();
		typeElement *app_sink;
		unsigned num_users;	
		bool is_run;
};

//GstFlowReturn bufferFromSinkCB (GstElement *elt, gpointer data);
gboolean onSinkMessage (GstBus *bus, GstMessage *message, gpointer data);
bool openCVStaticPipelineMake(OPELGstElementTx1 *tx1,
		    std::vector<typeElement*>*_type_element_vector);
#endif /*OPEL_RAW_REQUEST_H*/
