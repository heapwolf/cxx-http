{
  'includes': [ 'common.gypi' ],
  'targets': [
    {
      'target_name': 'server',
      'type': 'executable',
      'sources': [
        'server.cc',
      ],
      'include_dirs': [
        './deps/debug'
      ],
      'dependencies': [
        './deps/libuv/uv.gyp:libuv',
        './deps/http-parser/http_parser.gyp:http_parser'
      ],
    },
  ],
}
