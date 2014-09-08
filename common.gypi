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
        'cflags': [ 
          '-O3',
          '-std=c++1y', 
          '-fstrict-aliasing', 
          '-fomit-frame-pointer',
          '-fdata-sections',
          '-ffunction-sections',
          '-fPIC',
        ],
        'defines': [ 'NDEBUG' ],
        'xcode_settings': {
          'ALWAYS_SEARCH_USER_PATHS': 'NO',
          'GCC_CW_ASM_SYNTAX': 'NO',
          'GCC_DYNAMIC_NO_PIC': 'NO',
          'GCC_ENABLE_CPP_RTTI': 'NO',
          'GCC_ENABLE_PASCAL_STRINGS': 'NO',
          'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES',
          'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',
          'GCC_THREADSAFE_STATICS': 'NO',
          'GCC_WARN_ABOUT_MISSING_NEWLINE': 'YES',
          'PREBINDING': 'NO',
          'USE_HEADERMAP': 'NO',
          'OTHER_CPLUSPLUSFLAGS' : [ 
            '-O3', 
            '-std=c++1y', 
            '-fstrict-aliasing', 
            '-fomit-frame-pointer',
            '-fdata-sections',
            '-ffunction-sections',
            '-fPIC',
          ],
          'OTHER_CFLAGS': [ 
            '-O3', 
            '-fstrict-aliasing', 
            '-fomit-frame-pointer',
            '-fdata-sections',
            '-ffunction-sections',
            '-fPIC',
          ]
        }
      }
    }
  }
}

