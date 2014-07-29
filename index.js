var express = require('express'),
    http = require('http'),
    io = require('socket.io')
    app = express();

var server = http.createServer(app);
server.listen(5000);

app.use(express.static(__dirname + '/public'));

var opticalflow = require('./build/Release/opticalflow').opticalflow;

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
