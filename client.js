var socket = require('socket.io-client')("ws://localhost:5001");

socket.on('message', function(data) {
  console.log('vector length: ' + data.vector.length);
  console.log(data.vector);
  console.log('expect image: ' + data.expect_image);
  console.log('target image: ' + data.target_image);
  console.log('span: ' + data.span);
  console.log('threshold: ' + data.threshold);
  console.log('time: ' + data.time);
  console.log('status: ' + data.status);
});
socket.on('error', function(err) {
  console.log('error');
  console.log(err);
});
socket.on('finish', function(report) {
  console.log('report');
  console.log(report);
});
socket.on('disconnect', function() {
  console.log('disconnect');
});

socket.send({
  'expect_path': 'public/test_images/widget_common_normal',
  'target_path': 'public/test_images/widget_common_normal_scratch',
  'options': {
    'threshold': 5.0,
    'span': 10,
    'pyrScale':0.5,
    'pyrLevels': 3,
    'winSize': 30,
    'pyrIterations': 3,
    'polyN': 7,
    'polySigma': 1.5,
    'flags': 256
  }
});
