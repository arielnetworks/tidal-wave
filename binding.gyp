{
  "targets": [
    {
      "target_name": "tidalwave",
      "sources": [ 
        "src/main.cpp",
        "src/broker.cpp",
        "src/opticalflow.cpp",
        "src/manager.cpp",
        "src/consumer.cpp",
        "src/thread.cpp"
      ],
      "cflags_cc": [ "<!@(pkg-config --cflags opencv)" ],
      "libraries": [ 
        "<!@(pkg-config --libs opencv)",
        "-L/usr/local/cuda-6.0/lib64"
      ],
      "conditions": [
        ['OS=="mac"', {
          # cflags on OS X are stupid and have to be defined like this
          'xcode_settings': {
            'OTHER_CFLAGS': [
                "-mmacosx-version-min=10.7",
                "-std=c++11",
                "-stdlib=libc++",
              '<!@(pkg-config --cflags opencv)'
            ]
            , "GCC_ENABLE_CPP_RTTI": "YES"
            , "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
          }
        }]
      ]
    }
  ]
}
