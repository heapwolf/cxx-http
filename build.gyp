{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'libuv-http',
      'type': 'none',
      'sources': [
        'http.h',
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
        'test.cc',
      ],
      'dependencies': [
        './deps/libuv/uv.gyp:libuv',
        './deps/http-parser/http_parser.gyp:http_parser'
      ],
    },
  ],

}
