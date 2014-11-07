var express = require('express'),
  http = require('http'),
  io = require('socket.io'),
  app = express();

var TidalWave = require('./index.js');

var server = http.createServer(app);
server.listen(process.env.PORT || 5001);

app.use(express.static(__dirname + '/public'));


var socket = io.listen(server);
var busy = false;

socket.on('connection', function (client) {
  console.log('connection');

  client.on('message', function (data) {
    console.log(data);

    var tidalwave = TidalWave.create(
      data.expect_path,
      data.target_path,
      data.options
    );

    tidalwave.on('data', function (msg) {
      console.log("message vector length: " + msg.vector.length);
      //console.log(msg);
      client.send(msg);
    });

    tidalwave.on('error', function (err) {
      console.log("error!!");
      console.log(err);
      client.emit('error', err);
    });

    tidalwave.once('finish', function (report) {
      console.log("finish!!");
      console.log(report);
      client.emit('finish', report);
      client.disconnect();

      tidalwave.removeAllListeners();
    });

  });
});
