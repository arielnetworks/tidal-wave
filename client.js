var socket = require('socket.io-client')("ws://localhost:5000");

socket.on('message', function(data) {
  console.log(data.length());
});

socket.send({
  'expect_path': 'public/images',
  'target_path': 'public/images2',
  'threshold': 5,
  'span': 10
});
