var EventEmitter = require('events').EventEmitter
  , opt = require('./build/Debug/opticalflow');
//  , opt = require('./build/Release/opticalflow');

opt.OpticalFlow.prototype.__proto__ = EventEmitter.prototype;
module.exports = opt.OpticalFlow;

