#include "OPELglobalRequest.h"
OPELGlobalVectorRequest *OPELGlobalVectorRequest::global_vector_request = NULL;

OPELGlobalVectorRequest *OPELGlobalVectorRequest::getInstance(void)
{
  if(global_vector_request == NULL)
    global_vector_request = new OPELGlobalVectorRequest();
  return global_vector_request;
}
OPELGlobalVectorRequest::~OPELGlobalVectorRequest()
{
  
  if(this->v_request != NULL)
  {  
    if(this->v_request->size() != 0)
      OPEL_DBG_WARN("OPEL Global Vector has redundant element");
    delete this->v_request;
  }
}
OPELGlobalVectorRequest::OPELGlobalVectorRequest()
{ 
  this->v_request = new std::vector<OPELRequestTx1*>();
}
void OPELGlobalVectorRequest::pushRequest(OPELRequestTx1* _request)
{
  assert(this->v_request != NULL && _request != NULL);
  this->v_request->push_back(_request);
}

OPELRequestTx1* OPELGlobalVectorRequest::getRequestByPid(unsigned _pid)
{
   assert(this->v_request != NULL);
    
   OPELRequestTx1* iter = NULL;
   for(int i=0; i < this->v_request->size(); i++)
   {
     iter = (*v_request)[i];
     if(iter->getMsgHandle()->pid == _pid)
      return iter;
   }
   return NULL;
}

void OPELGlobalVectorRequest::deleteRequest(unsigned _pid)
{
  assert(this->v_request != NULL);
  
  OPELRequestTx1* iter = NULL;
  std::vector<OPELRequestTx1*>::iterator begin = v_request->begin();
 
  for(int i=0; i<this->v_request->size(); i++)
  {
    iter = (*this->v_request)[i];
    if(iter->getMsgHandle()->pid == _pid)
      v_request->erase(begin+i);
  }
  OPEL_DBG_WARN("Delete Request : Do Not Reach Here");
}

void OPELGlobalVectorRequest::deleteRequest(OPELRequestTx1 *_request)
{
  assert(this->v_request != NULL);
  
  OPELRequestTx1* iter = NULL;
  std::vector<OPELRequestTx1*>::iterator begin = v_request->begin();
 
  for(int i=0; i<this->v_request->size(); i++)
  {
    iter = (*this->v_request)[i];
    if(iter == _request)
      v_request->erase(begin+i);
  }
  OPEL_DBG_WARN("Delete Request : Do Not Reach Here");
}

void OPELGlobalVectorRequest::deleteRequest(dbusRequest* _request)
{
  assert(this->v_request != NULL);
  
  OPELRequestTx1* iter = NULL;
  std::vector<OPELRequestTx1*>::iterator begin = v_request->begin();
 
  for(int i=0; i<this->v_request->size(); i++)
  {
    iter = (*this->v_request)[i];
    if(iter->getMsgHandle() == _request)
      v_request->erase(begin+i);
  }
  OPEL_DBG_WARN("Delete Request : Do Not Reach Here");
}

