{
  'targets': [
    {
      'target_name': 'app-api',
      'sources': [
        'AppAPI.cc',
        'AppAPIInternal.cc',
        'AppBase.cc'
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
