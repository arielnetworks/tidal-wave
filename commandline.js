#!/usr/bin/env node
var usage =
  "[USAGE]\n" +
  "  node commandline.js [options] expect_path target_path\n" +
  "\n" +
  "ex)\n" +
  "  $ node commandline.js -threshold 1.0 -span 10 test/images test/images2\n";

var supportedOptions = [
  'threshold',
  'span',
  'pyrScale',
  'pyrLevels',
  'winSize',
  'pyrIterations',
  'polyN',
  'polySigma',
  'flags',
];

function suppressCout() {
  var util = require('util');
  var fs = require('fs');
  var dup2 = require('node-dup2');

  var stdout = fs.openSync("/dev/null", "a"); // dummy
  dup2.invoke(1, stdout);

  var devNull = fs.openSync("/dev/null", "a");
  dup2.invoke(devNull, 1);

  function printToStdout(s) {
    fs.write(stdout, JSON.stringify(s, null, '  ') + "\n");
  }

  console.__defineGetter__('log', function(){ return printToStdout; });
  console.__defineGetter__('error', function(){ return printToStdout; });
  console.__defineGetter__('info', function(){ return printToStdout; });
}

function getArgs() {
  var minimist = require('minimist');

  var args = minimist(process.argv.slice(2));
  var expect_path = args._[0];
  var target_path = args._[1];
  var options = {};
  supportedOptions.forEach(function(s){ options[s] = args[s] });

  if (!expect_path || !target_path) {
    console.error(usage);
    process.exit(-1);
  }

  return {
    expect_path: expect_path,
    target_path: target_path,
    options: options };
}

function main(expect_path, target_path, options) {
  var OpticalFlow = require('./opticalflow.js');

  suppressCout(); // suppress log messages using cout in OpticalFlow C++ module

  var opticalflow = new OpticalFlow();
  opticalflow.on('message', function(msg) { console.log(msg); });
  opticalflow.on('error', function(err) { console.error(err); });
  opticalflow.once('finish', function finish(report) { console.info(report); });
  opticalflow.calc(expect_path, target_path, options);
}

var args = getArgs();
main(args.expect_path, args.target_path, args.options);
