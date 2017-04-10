#ifndef OPEL_GLOBAL_REQUEST_H
#define OPEL_GLOBAL_REQUEST_H
#include "OPELgstElementTx1.h"

class OPELGlobalVectorRequest{
  public:
    //static OPELGlobalVectorRequest* getInstance(void);
    void pushRequest(OPELRequestTx1*);
    void deleteRequest(unsigned);
    void deleteRequest(OPELRequestTx1*);
    void deleteRequest(dbusRequest*);
    bool isVectorEntryEmpty(void);
    OPELRequestTx1* getRequestByPid(unsigned);
    ~OPELGlobalVectorRequest();
    OPELGlobalVectorRequest();

  private:
    //static OPELGlobalVectorRequest *global_vector_request;
    //OPELGlobalVectorRequest();
    std::vector<OPELRequestTx1*>* v_request;
};


#endif /* OPEL_GLOBAL_REQUEST_H */
