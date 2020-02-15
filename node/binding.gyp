{
    "targets": [{
        "target_name": "cloakfile",
        "sources": [
            "../cpp/src/node_export.cpp"
        ],
        'cflags!': [ '-fno-exceptions' ],
        'cflags_cc!': [ '-fno-exceptions' ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        'libraries': [
            "/usr/local/lib/libclk.so"
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'conditions': [
            ['OS=="mac"', {
            'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
            }
            }]
        ]
    }]
}