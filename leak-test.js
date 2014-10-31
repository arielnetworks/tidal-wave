//var heapdump = require('heapdump')

var OpticalFlow = require('./index.js');


var opticalflow = new OpticalFlow();

opticalflow.on('message', function(data) {
  console.log('message');
  /*
  console.log('vector length: ' + data.vector.length);
  console.log('expect image: ' + data.expect_image);
  console.log('target image: ' + data.target_image);
  console.log('span: ' + data.span);
  console.log('threshold: ' + data.threshold);
  console.log('time: ' + data.time);
  console.log('status: ' + data.status);
  */
});

opticalflow.on('finish', function() {
  console.log('finish');
});

setInterval(function() {
  opticalflow.calc(
    "public/test_images/widget_common_normal", 
    "public/test_images/widget_common_normal_scratch",
    { 'threshold': 3, "span": 2}
  );
  //heapdump.writeSnapshot();
}, 10000);


