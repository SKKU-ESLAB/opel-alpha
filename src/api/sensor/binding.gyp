{
  'targets': [
    {
      'target_name': 'sensor-api',
      'sources': [
        'nil.cc',
        'nil_dbus_connection.cc',
		    'nil_dbus.cc',
		    'nil_request.cc',
		    'nil.h',
        'cJSON.h',
        'cJSON.c',
        'sensfw_log.h'
       ],
      'libraries': [
        '<!@(pkg-config glib-2.0 --libs)',
        '<!@(pkg-config dbus-1 --libs)'
      ],
      'include_dirs': [
        '<!@(pkg-config glib-2.0 --cflags-only-I | sed s/-I//g)',
        '<!@(pkg-config dbus-1 --cflags-only-I | sed s/-I//g)'
      ]
    }
  ]
}


