{
  "targets": [
    {
      "target_name": "opticalflow",
      "sources": [ 
        "main.cpp",
        "opticalflow.cpp",
        "listfiles.cpp",
        "dispatcher.cpp"
      ],
      "cflags_cc": [ "<!@(pkg-config --cflags opencv)" ],
      "libraries": [ 
        "<!@(pkg-config --libs opencv)",
        "-L/usr/local/cuda-6.0/lib64"
      ],
    }
  ]
}
