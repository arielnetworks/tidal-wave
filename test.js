var OpticalFlow = require('./opticalflow.js');


var opticalflow = new OpticalFlow();

opticalflow.on('message', function(data) {
  console.log('vector length: ' + data.vector.length);
  console.log('expect image: ' + data.expect_image);
  console.log('target image: ' + data.target_image);
  console.log('span: ' + data.span);
  console.log('threshold: ' + data.threshold);
  console.log('time: ' + data.time);
  console.log('status: ' + data.status);
});

opticalflow.on('finish', function() {
  console.log('finish');
});
opticalflow.calc("public/images", "public/images2", 10, 5);
