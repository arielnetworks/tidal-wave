var socket = require('socket.io-client')("ws://localhost:5000");

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
socket.on('disconnect', function() {
  console.log('disconnect');
});

socket.send({
  'expect_path': 'public/images',
  'target_path': 'public/images2',
  'threshold': 5,
  'span': 10
});
