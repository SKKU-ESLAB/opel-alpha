{
  'targets': [
    {
      'target_name': 'app-api',
      'sources': [
        'AppAPI.cc',
        'AppAPIInternal.cc',
        'AppBase.cc'
       ],
      'link_settings': {
        'libraries': [
          '<!@(pkg-config glib-2.0 --libs)',
          '<!@(pkg-config dbus-1 --libs)',
          "-L<(PRODUCT_DIR)/../../../../out/libs -lopel-cmfw",
          "-L<(PRODUCT_DIR)/../../../../out/libs -lopel-message",
        ],
        'ldflags': [
          '-Wl,-rpath'
        ]
      },
      'include_dirs': [
        '<!@(pkg-config glib-2.0 --cflags-only-I | sed s/-I//g)',
        '<!@(pkg-config dbus-1 --cflags-only-I | sed s/-I//g)',
        '../../framework/message/inc',
        '../../framework/communication/inc'
      ]
    }
  ]
}
