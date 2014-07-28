var addon = require('./build/Release/opticalflow').opticalflow;

addon("customjsp1.png", "customjsp2.png", function(msg){
  console.log(msg.length);
});

console.log("hello");
