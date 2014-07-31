var socket = require('socket.io-client')("ws://localhost:5000");

socket.on('message', function(data) {
  console.log(data);
});

socket.send({
  'expect_path': 'customjsp1.png',
  'target_path': 'customjsp2.png',
  'threshold': 5,
  'span': 10
});
