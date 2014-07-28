{
  "targets": [
    {
      "target_name": "opticalflow",
      "sources": [ "opticalflow.cc" ],
      "cflags_cc": [ "<!@(pkg-config --cflags opencv)" ],
      "libraries": [ 
        "<!@(pkg-config --libs opencv)",
        "-L/usr/local/cuda-6.0/lib64"
      ],
    }
  ]
}
