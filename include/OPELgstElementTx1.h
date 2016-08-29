#ifndef OPEL_GST_ELEMENT_TX1_H
#define OPEL_GST_ELEMENT_TX1_H

#include "OPELgstElement.h"

const char *element_name_tx1[] = {"pipeline", "nvcamerasrc", "nvvidconv",
  "omxh264enc", "mp4mux", "filesink"};

const char *element_name_tx1_rtsp[] = {"pipeline", "rtspsrc", "nvvidconv",
  "omxh264enc", "mp4mux", "filesink"};



const char *element_caps_tx1_src[] = {"video/x-raw(memory:NVMM)", 
  "width", "height", "framerate"};
const char *element_caps_tx1_src_rtsp[] ={};
const char *element_caps_tx1_conv[] = {"video/x-raw(memory:NVMM)",
  "width", "height", "framerate"};
const char *element_caps_tx1_enc[] = {"video/x-h264", "stream-format"};


class OPELGstElementTx1 : public OPELGstElement
{
  public: 
    ~OPELGstElementTx1();
    static OPELGstElementTx1* getInstance(void);
    virtual bool OPELGstElementFactory(void);
    virtual bool OPELGstPipelineMake(void);
    virtual bool OPELGstElementCapFactory(void);
    virtual bool OPELGstElementPropFactory(void);
    void setIsRtspSrc(bool _is_rtsp_src);
    bool getIsRtspSrc(void) const;
  private:
    static OPELGstElementTx1 *opel_gst_element_tx1;
    bool is_rtsp_src;
    OPELGstElementTx1();
    GstCaps **caps_array;
    GMainLoop *main_loop;
};


#endif /* OPEL_GST_ELEMENT_TX1_H */
