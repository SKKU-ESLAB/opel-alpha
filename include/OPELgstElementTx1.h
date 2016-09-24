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
};

class OPELGstElementTx1 : public OPELGstElement
{
  public: 
    ~OPELGstElementTx1();
    static OPELGstElementTx1* getInstance(void);
    virtual bool OPELGstElementFactory(void);
    virtual bool OPELGstPipelineMake(void);
    virtual bool OPELGstElementCapFactory(void);
    virtual bool OPELGstElementPropFactory(void);
    virtual void setElementPropertyVector(std::vector<ElementProperty*>
                *__v_element_property);

    GstElement* getPipeline(void){ return this->pipeline; }
    void setPipeline(GstElement* _pipeline){ this->pipeline = _pipeline; }

  	bool getIsPlaying(void) const
		{ return this->is_playing; }
		void setIsPlaying(bool _is_playing) 
		{ this->is_playing = _is_playing; }
  private:
    static OPELGstElementTx1 *opel_gst_element_tx1;
    bool is_playing;
		OPELGstElementTx1();
    GstElement *pipeline;
    GstCaps *src2conv;
};

static bool gstElementFactory(std::vector<typeElement*> 
    *_type_element_vector);
static void gstElementPropFactory(std::vector<typeElement*>
    *_type_element_vector);
static void freeTypeElementMember(typeElement *type_element);
static void initializeTypeElement(typeElement *type_element,
        ElementProperty *element_property);
void printTypeElement(std::vector<typeElement*> *_type_element_vector);

#endif /* OPEL_GST_ELEMENT_TX1_H */
