var EventEmitter = require('events').EventEmitter,
  TidalWave = require('./build/Release/tidalwave').TidalWave,
  globy = require('globy'),
  _ = require('underscore');
var path = require("path");
_.str = require('underscore.string');
_.mixin(_.str.exports());

TidalWave.prototype.__proto__ = EventEmitter.prototype;

TidalWave.prototype.calcAll = function (expect_dir, target_dir) {

  if (!_.endsWith(expect_dir, '/')) {
    expect_dir += '/';
  }
  if (!_.endsWith(target_dir, '/')) {
    target_dir += '/';
  }

  var target_files = globy.glob(target_dir + '**/*.*');

  var self = this;

  var count = 0;
  _.each(target_files, function (target_file) {
    var expected_file = target_file.replace(target_dir, expect_dir);
    if (path.existsSync(expected_file)) {
      self.calc(expected_file, target_file);
      count++;
    }
  });

  return count;
};


module.exports = TidalWave;

