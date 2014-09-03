{
  'variables': {
    'conditions': [
      ['OS == "mac"', {
        'target_arch%': 'x64'
      }, {
        'target_arch%': 'ia32'
      }]
    ]
  },
  'target_defaults': {
    'default_configuration': 'Release',
    'defines': [ 'HTTP_PARSER_STRICT=0' ],
    'conditions': [
      ['OS == "mac"', {
        'defines': [ 'DARWIN' ]
      }],
	  ['OS == "linux"', {
        'defines': [ 'LINUX' ]
      }],
	  ['OS == "win"', {
        'defines': [ 'WIN32' ]
      }],
      ['OS == "mac" and target_arch == "x64"', {
        'xcode_settings': {
          'ARCHS': [ 'x86_64' ]
        },
      }]
    ],
    'configurations': {
      'Debug': {
        'cflags': [ '-g', '-O0', '-std=c++1y' ],
        'defines': [ 'DEBUG' ],
        'xcode_settings': {
          'OTHER_CPLUSPLUSFLAGS' : [ '-std=c++1y' ],
          'OTHER_LDFLAGS': ['-std=c++1y'],
          'OTHER_CFLAGS': [ '-g', '-O0' '-Wall' ]
        }
      },
      'Release': {
        'cflags': [ '-O3', '-std=c++1y'],
        'defines': [ 'NDEBUG' ],
        'xcode_settings': {
          'OTHER_CPLUSPLUSFLAGS' : [ '-O3', '-std=c++1y'],
          'OTHER_CFLAGS': [ '-O3' ]
        }
      }
    }
  }
}
