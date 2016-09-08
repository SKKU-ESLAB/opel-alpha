#ifndef OPEL_GST_ELEMENT_TX1_H
#define OPEL_GST_ELEMENT_TX1_H

#include "OPELgstElement.h"

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

   
  private:
    static OPELGstElementTx1 *opel_gst_element_tx1;
    OPELGstElementTx1();
    GstElement *pipeline;
};


static void freeTypeElementMember(typeElement *type_element);
static void initializeTypeElement(typeElement *type_element,
        ElementProperty *element_property);
void printTypeElement(std::vector<typeElement*> *_type_element_vector);
#endif /* OPEL_GST_ELEMENT_TX1_H */
