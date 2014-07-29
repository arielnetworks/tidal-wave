var socket = require('socket.io-client')("ws://localhost:5000");

socket.on('message', function(data) {
  console.log(data);
});

socket.send({
  'expect_image': 'customjsp1.png',
  'target_image': 'customjsp2.png'
});
