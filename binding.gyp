{
  "targets": [
    {
      "target_name": "opticalflow",
      "sources": [ 
        "src/main.cpp",
        "src/broker.cpp",
        "src/opticalflow.cpp",
        "src/utility.cpp",
        "src/manager.cpp",
        "src/producer.cpp",
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
