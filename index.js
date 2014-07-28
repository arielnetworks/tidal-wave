var http = require('http'),
    io = require('socket.io');

var opticalflow = require('./build/Release/opticalflow').opticalflow;


console.log("hello");

var server = http.createServer(function(req, res) {
  res.writeHead(200, {'Content-Type': 'text/html'});
  res.write('<h1>テスト</h1>');
  res.end();
});
server.listen(5000);

var socket = io.listen(server);

socket.on('connection', function(client) {
  console.log('connection');
  client.on('message', function(data) {
    console.log(data);
    opticalflow(data.expect_image, data.target_image, function(msg){
      console.log(msg.length);
      client.send(msg);
    });
  });
});
