var EventEmitter = require('events').EventEmitter,
  TidalWave = require('./build/Release/tidalwave').TidalWave,
  glob = require('glob-stream'),
  Path = require('path');
TidalWave.prototype.__proto__ = EventEmitter.prototype;



module.exports.create = create;



function create(expect_dir, target_dir, options) {
  var t = new TidalWave(options);
  calcAll(t, expect_dir, target_dir);
  return t;
}

function calcAll(tidalwave, expect_dir, target_dir) {
  var globEnded = false;
  var requested = 0;

  var stream = glob.create(Path.resolve(target_dir, '**/*.*'));
  stream.once('end', function() {
    globEnded = true;
  });
  stream.on('data', function(target) {
    var file = target.path.substr(target.base);
    var expected_file = Path.resolve(expect_dir, file);
    Path.exists(expected_file, function(exists) {
      tidalwave.calc(expected_file, target.path);
      requested++;
    });
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
