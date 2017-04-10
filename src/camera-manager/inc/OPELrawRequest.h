#ifndef OPEL_RAW_REQUEST_H
#define OPEL_RAW_REQUEST_H

#include "OPELgstElement.h"
#include "OPELgstElementTx1.h"
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <semaphore.h>

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

    bool initializeSemAndSharedMemory(void);

    unsigned getNumUsers(void) const
    { return this->num_users; }

    void setNumUsers(unsigned _num_users)
    { this->num_users = _num_users; }

    void increaseNumUsers(void)
    { this->num_users++; }
    void decreaseNumUsers(void)
    { this->num_users--; }

    bool getIsRun(void) const
    { return this->is_run; }
    void setIsRun(bool _is_run)
    { this->is_run = _is_run; }

    typeElement* getAppSink(void)
    {  return this->app_sink; }

    sem_t* getSemaphore(void)
    { return this->sem; }

    int getShmId(void) 
    { return this->shm_id; }

    char* getShmPtr(void)
    { return (char*)this->shm_ptr; }

    void* getBufferPtr(void)
    {
      return this->shm_ptr;
    }
    char* getBufferSizePtr(void)
    {
      char *tmp = (char*)this->shm_ptr;
      return tmp + RAW_DEFAULT_BUF_SIZE;
    }
    void setGstMainTeePad(GstPad *_main_tee_pad)
    {
      this->main_tee_pad = _main_tee_pad;
    }
    GstPad* getGstMainTeePad(void)
    { return this->main_tee_pad; }
    bool detachedOpenCVPipeline(void);

  private:
    static OPELRawRequest *opel_raw_request;
    OPELRawRequest();
    typeElement *app_sink;

    GstPad *main_tee_pad;

    int shm_id;
    void *shm_ptr;
    sem_t *sem;

    unsigned num_users;
    bool is_run;

};

//GstFlowReturn bufferFromSinkCB (GstElement *elt, gpointer data);
gboolean onSinkMessage (GstBus *bus, GstMessage *message, gpointer data);
bool openCVStaticPipelineMake(OPELGstElementTx1 *tx1,
    std::vector<typeElement*>*_type_element_vector);

static int uinitSharedMemorySpace(int _shm_id);
static int initSharedMemorySpace(int _req_count, int _buffer_size,
    void** _shm_ptr, key_t _shmkey);
static bool initSemaphore(const char *path, sem_t **_sem);
static void uinitSemaphore(const char *path, sem_t *_sem);

#endif /*OPEL_RAW_REQUEST_H*/
