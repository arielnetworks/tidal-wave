var EventEmitter = require('events').EventEmitter,
    OpticalFlow = require('./build/Release/opticalflow').OpticalFlow,
    globy = require('globy'),
    _ = require('underscore');
var path = require("path");
_.str = require('underscore.string');
_.mixin(_.str.exports());

OpticalFlow.prototype.__proto__ = EventEmitter.prototype;

OpticalFlow.prototype.calcAll = function (expect_dir, target_dir) {

    if (!_.endsWith(expect_dir, '/')) {
        expect_dir += '/';
    }
    if (!_.endsWith(target_dir, '/')) {
        target_dir += '/';
    }

    var target_files = globy.glob(target_dir + '**/*.png');

    var self = this;

    _.each(target_files, function(target_file){
      var expected_file = target_file.replace(target_dir, expect_dir);
      if (path.existsSync(expected_file)) {
        self.calc(expected_file, target_file);
      }
    });
};


module.exports = OpticalFlow;

