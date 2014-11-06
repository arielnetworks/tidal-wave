var TidalWave = require('./index.js');

var tidalwave = TidalWave.create(
    'test/fixture/expected',
    'test/fixture/revision1', {
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

tidalwave.on('data', console.log.bind(null, '--onData--'));
tidalwave.on('error', console.log.bind(null, '--onError--'));
tidalwave.on('finish', console.log.bind(null, '--onFinish--'));
