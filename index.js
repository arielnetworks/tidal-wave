var express = require('express'),
    http = require('http'),
    io = require('socket.io')
    app = express();

var OpticalFlow = require('./opticalflow.js');

var server = http.createServer(app);
server.listen(5555);

app.use(express.static(__dirname + '/public'));


var socket = io.listen(server);
var opticalflow = new OpticalFlow();
var busy = false;

socket.on('connection', function(client) {
  console.log('connection');
  if(busy) {
    console.log("busy!!!!!!");
    client.disconnect();
    return;
  }
  busy = true;

  client.on('message', function(data) {
    console.log(data);
    var onMessage = function(msg) {
      //console.log("message vector length: " + msg.vector.length);
      console.log(msg);
      client.send(msg);
    };
    opticalflow.on('message', onMessage);
    opticalflow.once('finish', function() {
      console.log("finish!!");
      client.disconnect();
      busy = false;
      opticalflow.removeListener('message', onMessage);
    });
    opticalflow.calc(
      data.expect_path,
      data.target_path,
      data.options
    );
  });
});
