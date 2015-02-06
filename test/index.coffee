
TidalWave = require("../index")
assert = require("assert")
Path = require("path")



describe "TidalWave", ->

  @timeout 3 * 1000

  it "should report nothing on \"revision1\"", (done) ->
    t = createWithExpectDir("revision1")
    t.on "data", (data) ->
      delete data.time
      if ~data.target_image.indexOf("capture1")
        assert.deepEqual data, {
          status: 'OK',
          span: 10,
          threshold: 5,
          expect_image: Path.join(__dirname, 'fixture/expected/scenario1/capture1.jpg'),
          target_image: Path.join(__dirname, 'fixture/revision1/scenario1/capture1.jpg'),
          vector: []
        }
      else if ~data.target_image.indexOf("capture2")
        assert.deepEqual data, {
          status: 'OK',
          span: 10,
          threshold: 5,
          expect_image: Path.join(__dirname, 'fixture/expected/scenario2/capture2.png'),
          target_image: Path.join(__dirname, 'fixture/revision1/scenario2/capture2.png'),
          vector: []
        }
      else
        assert.fail "Cannot be here."
    t.on "finish", (report) ->
      assert.deepEqual report, { request: 2, data: 2, error: 0 }
      done()

  it "should report something on \"revision2\"", (done) ->
    t = createWithExpectDir("revision2")
    t.on "data", (data) ->
      delete data.time
      if ~data.target_image.indexOf("capture1")
        assert.deepEqual data, {
          status: 'OK',
          span: 10,
          threshold: 5,
          expect_image: Path.join(__dirname, 'fixture/expected/scenario1/capture1.jpg'),
          target_image: Path.join(__dirname, 'fixture/revision2/scenario1/capture1.jpg'),
          vector: [] }
      else if ~data.target_image.indexOf("capture2")
        assert.deepEqual data, {
          status: 'SUSPICIOUS',
          span: 10,
          threshold: 5,
          expect_image: Path.join(__dirname, 'fixture/expected/scenario2/capture2.png'),
          target_image: Path.join(__dirname, 'fixture/revision2/scenario2/capture2.png'),
          vector: [
             { x: 80, y: 70, dx: -5.360568046569824, dy: -0.0551748163998127 },
             { x: 130, y: 70, dx: -6.001735687255859, dy: -1.3181204795837402 },
             { x: 140, y: 70, dx: -71.92633819580078, dy: -0.5746171474456787 },
             { x: 170, y: 70, dx: -5.842303276062012, dy: -1.6100093126296997 },
             { x: 110, y: 80, dx: 6.5268402099609375, dy: 1.0273534059524536 },
             { x: 130, y: 80, dx: -18.383054733276367, dy: 1.066022515296936 },
             { x: 140, y: 80, dx: -88.46060180664062, dy: 0.5541346073150635 },
             { x: 160, y: 80, dx: -6.80324125289917, dy: 1.1075118780136108 },
             { x: 170, y: 80, dx: -8.922819137573242, dy: 0.7354532480239868 },
             { x: 160, y: 90, dx: -5.266021728515625, dy: 2.406721591949463 },
             { x: 170, y: 90, dx: -5.1159467697143555, dy: 2.25892972946167 },
             { x: 90, y: 100, dx: 0.7105715870857239, dy: -7.616470813751221 },
             { x: 100, y: 100, dx: 2.2438039779663086, dy: -8.364863395690918 },
             { x: 110, y: 100, dx: 1.3381984233856201, dy: -5.666755676269531 },
             { x: 120, y: 100, dx: 0.940326452255249, dy: -6.6747002601623535 },
             { x: 160, y: 100, dx: -5.092167377471924, dy: -2.799497604370117 },
             { x: 170, y: 100, dx: -3.990016460418701, dy: -5.452290058135986 },
             { x: 80, y: 110, dx: -1.5481517314910889, dy: -5.242636680603027 },
             { x: 90, y: 110, dx: 0.8682486414909363, dy: -9.285847663879395 },
             { x: 100, y: 110, dx: 2.520627975463867, dy: -9.215091705322266 },
             { x: 110, y: 110, dx: 7.245147228240967, dy: -8.47363567352295 },
             { x: 120, y: 110, dx: 2.8622498512268066, dy: -9.007637023925781 },
             { x: 160, y: 110, dx: -6.2871503829956055, dy: -0.9457563161849976 },
             { x: 170, y: 110, dx: -7.390625476837158, dy: -5.659643173217773 } ] }
      else
        assert.fail "Cannot be here."
    t.on "finish", (report) ->
      assert.deepEqual report, { request: 2, data: 2, error: 0 }
      done()

  it "should never report on \"__NOT_EXISTS__\"", (done) ->
    t = createWithExpectDir("__NOT_EXISTS__")
    t.on "data", (data) ->
      assert.fail "boom."
    t.on "finish", (report) ->
      assert.deepEqual report, { request: 0, data: 0, error: 0 }
      done()

  it "should start with passing 'getExpectedPath' option", (done) ->
    counter = 0
    t = TidalWave.create(
      Path.resolve(__dirname, "./fixture/revision2"), {
        getExpectedPath: (shortPath)->
          return Path.resolve(__dirname, "./fixture/revision1", shortPath)
      })
    t.on "data", (data) -> counter++
    t.on "finish", (report) ->
      assert.equal counter, 2
      assert.deepEqual report, { request: 2, data: 2, error: 0 }
      done()

createWithExpectDir = (revisionBaseDir)->
  TidalWave.create(
      Path.resolve(__dirname, "./fixture/#{revisionBaseDir}"),
      {
        expectDir: Path.resolve(__dirname, "./fixture/expected")
      })

