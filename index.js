var EventEmitter = require('events').EventEmitter,
  TidalWave = require('./build/Release/tidalwave').TidalWave,
  glob = require('glob-stream'),
  Path = require('path'),
  FS = require('fs');
TidalWave.prototype.__proto__ = EventEmitter.prototype;



module.exports.create = create;



function create(target_dir, options) {
  var t = new TidalWave(options);
  options = options || {};

  var getExpectedPath;
  if (typeof options.expect_dir === 'string') {
    getExpectedPath = function(shortPath) {
      return Path.resolve(options.expect_dir, shortPath);
    };
  } else if (typeof options.getExpectedPath === 'function') {
    getExpectedPath = options.getExpectedPath;
  } else {
    throw new Error('You must pass "expect_dir" or "getExpectedPath" as an option.');
  }

  calcAll(t, target_dir, getExpectedPath);
  return t;
}

function calcAll(tidalwave, target_dir, getExpectedPath) {
  var fileExists = false;
  var globEnded = false;
  var requested = 0;

  var stream = glob.create(Path.resolve(target_dir, '**/*.*'));
  stream.once('end', function() {
    globEnded = true;
    if (!fileExists) {
      tidalwave.dispose();
    }
  });
  stream.on('data', function(target) {
    fileExists = true;
    var shortPath = Path.relative(target.base, target.path);
    var expected_file = getExpectedPath.call(this, shortPath);
    if (!expected_file) {
      return;
    } else if (typeof expected_file === 'string') {
      calcIfExists(expected_file)
    } else if (typeof expected_file.then === 'function') {
      expected_file.then(calcIfExists);
    }
    function calcIfExists(expected_file) {
      FS.exists(expected_file, function(exists) {
        tidalwave.calc(expected_file, target.path);
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
