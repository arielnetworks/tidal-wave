var OpticalFlow = require('../opticalflow');

var assert = require("assert");

describe('Opticalflow', function(){
  this.timeout(3 * 1000);

  it('should start calcuration', function(done){
    var o = new OpticalFlow;
    o.on('message', console.log);
    o.on('finish', done);
    o.calc('./fixture/expected', './fixture/revision1');
  });

});
