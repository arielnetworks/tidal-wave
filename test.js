var Tidalwave = require('./index.js');

var tidalwave = new Tidalwave(
  {
    'threshold': 5.0,
    'span': 10,
    'numThreads': 5,
    'pyrScale':0.5,
    'pyrLevels': 3,
    'winSize': 30,
    'pyrIterations': 3,
    'polyN': 7,
    'polySigma': 1.5,
    'flags': 256
  });

tidalwave.on('message', function(data) {
  console.log('message');
  console.log(data);
});

tidalwave.on('error', function(err) {
  console.log('error');
  console.log(err);
});

tidalwave.on('finish', function() {
  console.log('finish');
});

tidalwave.calcAll(
  'public/test_images/widget_common_normal',
  'public/test_images/widget_common_normal_scratch'
);
