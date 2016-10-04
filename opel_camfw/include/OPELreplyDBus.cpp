#include "OPELcamRequest.h"
#include <assert.h>
void sendReply(DBusConnection *_conn, DBusMessage *_reply, bool is_success)
{
	assert(_conn != NULL && _reply != NULL);
	unsigned _is_success = is_success;
	DBusMessage *reply = dbus_message_new_method_return(_reply);
	
	dbus_message_append_args(reply,
			DBUS_TYPE_UINT64, &_is_success,
			DBUS_TYPE_INVALID);
	dbus_connection_send(_conn, reply, NULL);
}
