var EventEmitter = require('events').EventEmitter,
  TidalWave = require('./build/Release/tidalwave').TidalWave,
  glob = require('glob-stream'),
  Path = require('path'),
  FS = require('fs');
TidalWave.prototype.__proto__ = EventEmitter.prototype;



module.exports.create = create;



function create(targetDir, options) {
  var t = new TidalWave(options);
  options = options || {};

  var getExpectedPath;
  if (typeof options.expectDir === 'string') {
    getExpectedPath = function(shortPath) {
      return Path.resolve(options.expectDir, shortPath);
    };
  } else if (typeof options.getExpectedPath === 'function') {
    getExpectedPath = options.getExpectedPath;
  } else {
    throw new Error('An option must have "expectDir" or "getExpectedPath" property.');
  }

  calcAll(t, targetDir, getExpectedPath);
  return t;
}

function calcAll(tidalwave, targetDir, getExpectedPath) {
  var fileExists = false;
  var globEnded = false;
  var requested = 0;

  var stream = glob.create(Path.resolve(targetDir, '**/*.*'));
  stream.once('end', function() {
    globEnded = true;
    if (!fileExists) {
      tidalwave.dispose();
    }
  });
  stream.on('data', function(target) {
    fileExists = true;
    var shortPath = Path.relative(target.base, target.path);
    var expectedFile = getExpectedPath.call(this, shortPath);
    if (!expectedFile) {
      return;
    } else if (typeof expectedFile === 'string') {
      calcIfExists(expectedFile)
    } else if (typeof expectedFile.then === 'function') {
      expectedFile.then(calcIfExists);
    }
    function calcIfExists(expectedFile) {
      FS.exists(expectedFile, function(exists) {
        tidalwave.calc(expectedFile, target.path);
        requested++;
      });
    }
  });

  tidalwave.on('data', function() {
    if (globEnded && --requested <= 0) {
      tidalwave.dispose();
    }
  });
  tidalwave.on('error', function() {
    requested--;
  });

}
