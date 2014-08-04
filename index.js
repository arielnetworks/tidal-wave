var express = require('express'),
    http = require('http'),
    io = require('socket.io')
    app = express();

var OpticalFlow = require('./opticalflow.js');

var server = http.createServer(app);
server.listen(5001);

app.use(express.static(__dirname + '/public'));


var socket = io.listen(server);

socket.on('connection', function(client) {
  console.log('connection');
  client.on('message', function(data) {
    console.log(data);

    var opticalflow = new OpticalFlow();
    opticalflow.on('message', function(msg) {
      console.log(msg.vector.length);
      client.send(msg);
    });
    opticalflow.on('finish', function() {
      client.disconnect();
    });
    opticalflow.calc(
      data.expect_path,
      data.target_path,
      data.threshold ? data.threshold : 5,
      data.span ? data.span : 10
    );
  });
});
