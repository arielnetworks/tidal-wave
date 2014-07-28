{
  "targets": [
    {
      "target_name": "opticalflow",
      "sources": [ 
        "opticalflow.cc",
        "farneback_optical_flow.cpp"
      ],
      "cflags_cc": [ "<!@(pkg-config --cflags opencv)" ],
      "libraries": [ 
        "<!@(pkg-config --libs opencv)",
        "-L/usr/local/cuda-6.0/lib64"
      ],
    }
  ]
}
