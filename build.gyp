{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'nodeuv-http',
      'product_name': 'nodeuv-http',
      'type': 'static_library',
      'sources': [
        'http.h', 'http.cc'
      ],
      'dependencies': [
        './deps/libuv/uv.gyp:libuv',
        './deps/http-parser/http_parser.gyp:http_parser'
      ],
    },
    {
      'target_name': 'test',
      'type': 'executable',
      'sources': [
        'http.cc', 'test.cc',
      ],
      'dependencies': [
        './deps/libuv/uv.gyp:libuv',
        './deps/http-parser/http_parser.gyp:http_parser',
        'nodeuv-http'
      ],
    },
  ],
}

