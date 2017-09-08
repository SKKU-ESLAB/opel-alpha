{
  'targets': [
    {
      'target_name': 'app-api',
      'sources': [
        'nil.cc',
        'nil_dbus_connection.cc',
        'nil_dbus.cc',
        'nil_request.cc',
        'nil_internal.cc',
        'nil.h',
        'nil_internal.h'
       ],
      'libraries': [
        '<!@(pkg-config glib-2.0 --libs)',
        '<!@(pkg-config dbus-1 --libs)',
        '-L../../out/lib/libopel-cmfw.so',
        '-L../../out/lib/libopel-message.so'
      ],
      'include_dirs': [
        '<!@(pkg-config glib-2.0 --cflags-only-I | sed s/-I//g)',
        '<!@(pkg-config dbus-1 --cflags-only-I | sed s/-I//g)',
        '../../framework/message/inc',
        '../../framework/communication/inc'
      ]
    }
  ]
}
