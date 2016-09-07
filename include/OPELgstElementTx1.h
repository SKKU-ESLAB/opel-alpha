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
  private:
    static OPELGstElementTx1 *opel_gst_element_tx1;
    OPELGstElementTx1();
    GstCaps **caps_array;
};


#endif /* OPEL_GST_ELEMENT_TX1_H */
