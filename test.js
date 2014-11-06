var Tidalwave = require('./index.js');

var tidalwave = new Tidalwave(
  {
    'threshold': 5.0,
    'span': 10,
    'numThreads': 5,
    'pyrScale': 0.5,
    'pyrLevels': 3,
    'winSize': 30,
    'pyrIterations': 3,
    'polyN': 7,
    'polySigma': 1.5,
    'flags': 256
  });

var msgCount = 0;
var reqCount = 0;

tidalwave.on('data', function (data) {
  console.log('data');
  console.log(data);
  msgCount++;

  if (msgCount === reqCount) {
    if (tidalwave) {
      console.log("dispose!!");
      tidalwave.dispose();
    }
  }
});

tidalwave.on('error', function (err) {
  msgCount++;
  console.log('error');
  console.log(err);
});

reqCount = tidalwave.calcAll(
  'test/fixture/expected',
  'test/fixture/revision1'
);

//reqCount = tidalwave.calcAll(
//  'public/test_images/widget_common_normal',
//  'public/test_images/widget_common_normal_scratch'
//);

tidalwave.on('finish', function () {
  console.log("reqCount: " + reqCount);
  console.log('finish!!!!!!!!!!!!!');
  tidalwave.removeAllListeners();
  tidalwave = null;

  setImmediate(function () {
    if (global.gc) {
      global.gc();
    }
  })
});

