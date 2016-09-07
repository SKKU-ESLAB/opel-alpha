#include "OPELcamRequest.h"
#include "OPELdbugLog.h"

DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *user_data)
{
  __OPEL_FUNCTION_ENTER__;
  
  dbusRequest *request = (dbusRequest*)user_data;
  
  if(dbus_message_is_signal(msg, dbus_interface, rec_init_request))
  {
     OPEL_DBG_VERB("Get Recording Initialization Request");
    /*     dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64,
       &(request->msg_type), DBUS_TYPE_UINT64, &(request->u.init_type), 
       DBUS_TYPE_UINT64, &(request->fps), DBUS_TYPE_UINT64, &(request->width),
       DBUS_TYPE_UINT64, &(request->height), DBUS_TYPE_UINT64, 
       &(request->num_frames), DBUS_TYPE_INVALID); */
  }

  if(dbus_message_is_signal(msg, dbus_interface, rec_start_request))
  {
     OPEL_DBG_VERB("Get Recording Start Request");
    /*     dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64,
       &(request->msg_type), DBUS_TYPE_UINT64, &(request->u.dynamic_type), 
       DBUS_TYPE_UINT64, &(request->fps), DBUS_TYPE_UINT64, &(request->width),
       DBUS_TYPE_UINT64, &(request->height), DBUS_TYPE_UINT64, 
       &(request->num_frames), DBUS_TYPE_INVALID); */
  }
  
  if(dbus_message_is_signal(msg, dbus_interface, rec_stop_request))
  {
     OPEL_DBG_VERB("Get Recording Start Request");
  
  
  }

  
    
  
  __OPEL_FUNCTION_EXIT__;
  return DBUS_HANDLER_RESULT_HANDLED;
}

