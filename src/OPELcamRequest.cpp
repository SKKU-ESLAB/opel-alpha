#include "OPELcamRequest.h"

static ElementProperty* initElementProperty(elementType type,
       const std::string &name, const std::string &nickname,
            unsigned fps, unsigned width, unsigned height, ...)
{
    __OPEL_FUNCTION_ENTER__;


    __OPEL_FUNCTION_EXIT__;
    return NULL; 
}

static DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *user_data)
{
  __OPEL_FUNCTION_ENTER__;
  if(user_data == NULL)
  { 

    __OPEL_FUNCTION_EXIT__;
    return DBUS_HANDLER_RESULT_HANDLED;
  }
  dbusRequest *request = (dbusRequest*)user_data;
  
  if(dbus_message_is_signal(msg, dbus_interface, init_request))
  {
     dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64,
       &(request->msg_type), DBUS_TYPE_UINT64, &(request->u.init_type), 
       DBUS_TYPE_UINT64, &(request->fps), DBUS_TYPE_UINT64, &(request->width),
       DBUS_TYPE_UINT64, &(request->height), DBUS_TYPE_UINT64, 
       &(request->num_frames), DBUS_TYPE_INVALID);
  }

  if(dbus_message_is_signal(msg, dbus_interface, start_request))
  {
     dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT64,
       &(request->msg_type), DBUS_TYPE_UINT64, &(request->u.dynamic_type), 
       DBUS_TYPE_UINT64, &(request->fps), DBUS_TYPE_UINT64, &(request->width),
       DBUS_TYPE_UINT64, &(request->height), DBUS_TYPE_UINT64, 
       &(request->num_frames), DBUS_TYPE_INVALID);
  }
  
  if(dbus_message_is_signal(msg, dbus_interface, stop_request))
  {

  }

  
  __OPEL_FUNCTION_EXIT__;
  return DBUS_HANDLER_RESULT_HANDLED;
}

