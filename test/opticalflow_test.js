var OpticalFlow = require('../opticalflow');

var path = require('path');
var assert = require("assert");
var _ = require("underscore");

var fixturePath = path.resolve(__dirname, './fixture');

describe('Opticalflow', function(){
  this.timeout(3 * 1000);

  it('starts calcuration on calling "calc()".', function(done){
    var o = new OpticalFlow;
    o.on('finish', function(data) { done() });
    o.calc(fixturePath + '/expected', fixturePath + '/revision1');
  });

  it('provides error when specified directory does not exist.', function(done){
    var o = new OpticalFlow;
    o.on('error', function(data) {
      for (var property in data) {
        var value = data[property];
        switch(property) {
          case 'status': assert(value == 'ERROR'); break;
          case 'reason': assert(_.isString(value)); break;
          default: assert.fail(); break;
        }
      }
      done();
    });
    o.calc(fixturePath + '/expected', fixturePath + '/__NOT_EXIST__');
  });

  it('contains such properties in data on "message".', function(done){
    var o = new OpticalFlow;
    var results = [];
    o.on('message', function(data) {
      for (var property in data) {
        var value = data[property];
        switch(property) {
          case 'span':
            assert(_.isNumber(value)); break;
          case 'threshold':
            assert(_.isNumber(value)); break;
          case 'expect_image':
            assert(_.isString(value)); break;
          case 'target_image':
            assert(_.isString(value)); break;
          case 'time':
            assert(_.isNumber(value)); break;
          case 'vector':
            assert(_.isArray(value)); break;
          case 'status':
            assert(_.isString(value)); break;
          default:
            assert.fail('boom'); break;
        }
      }
    });
    o.on('finish', function(data) { done() });
    o.calc(fixturePath + '/expected', fixturePath + '/revision1');
  });

  it('reports no conflict between "expected" and "revision1".', function(done){
    var o = new OpticalFlow;
    var results = [];
    o.on('message', function(data) { results.push(data) });
    o.on('finish', function() {
      assert(results.every(function(result) {
        return result.vector.length == 0;
      }));
      done();
    });
    o.calc(fixturePath + '/expected', fixturePath + '/revision1');
  });

  it('reports a conflict between "expected" and "revision2".', function(done){
    var o = new OpticalFlow;
    var results = [];
    o.on('message', function(data) { results.push(data) });
    o.on('finish', function() {
      results.forEach(function(result) {
        if (~result.expect_image.indexOf('capture1')) {
          assert(result.vector.length == 0);
        } else if (~result.expect_image.indexOf('capture2')) {
          assert(result.vector.length > 0);
        }
      });
      done();
    });
    o.calc(fixturePath + '/expected', fixturePath + '/revision2');
  });

});
