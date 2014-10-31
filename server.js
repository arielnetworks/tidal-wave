var express = require('express'),
    http = require('http'),
    io = require('socket.io')
    app = express();

var OpticalFlow = require('./index.js');

var server = http.createServer(app);
server.listen(process.env.PORT || 5001);

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
    opticalflow.on('error', function(err) {
      console.log("error!!");
      console.log(err);
      client.emit('error', err);
    });
    opticalflow.once('finish', function(report) {
      console.log("finish!!");
      console.log(report);
      client.emit('finish', report);
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
