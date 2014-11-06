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
      "defines": [
      ]
    }
  ]
}
