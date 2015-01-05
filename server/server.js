var spawn = require('child_process').spawn;
var fs = require('fs');
var mkdirp = require('mkdirp');
var Pend = require('pend');
var createGzipStatic = require('connect-static');
var path = require('path');
var http = require('http');
var yawl = require('yawl');

var httpPort = process.env.HTTP_PORT || 21119;
var httpHost = process.env.HTTP_HOST || '0.0.0.0';

function spawnNethack(cb) {
  var pend = new Pend();
  mkdirp('run/dumps', pend.hold());
  mkdirp('run/save', pend.hold());
  pend.wait(function(err) {
    if (err) return cb(err);
    fs.writeFile('run/perm', "", pend.hold());
    fs.writeFile('run/logfile', "", pend.hold());
    fs.writeFile('run/record', "", pend.hold());
    pend.wait(function(err) {
      if (err) return cb(err);
      var options = {
        stdio: ['pipe', 'pipe', process.stderr],
      };
      var child = spawn('build/nethack', [], options);
      cb(null, child);
    });
  });
}

createGzipStatic({dir: path.join(__dirname, "public")}, function(err, staticMiddleware) {
  if (err) throw err;
  var httpServer = http.createServer(handleResponse);
  var wss = yawl.createServer({
      server: httpServer,
      allowTextMessages: true,
      maxFrameSize: 200 * 1024, // 200 KB
      origin: null,
  });
  wss.on('error', function(err) {
    console.error("web socket server error:", err.stack);
  });
  wss.on('connection', function(ws) {
    console.info("TODO: new websocket connection");
  });
  httpServer.listen(httpPort, httpHost, function() {
    console.info("HTTP server listening at http://" + httpHost + ":" + httpPort + "/");
  });
  

  function handleResponse(request, response) {
    staticMiddleware(request, response, function(err) {
      if (err) {
        console.error(err.stack);
        return;
      }
      response.statusCode = 404;
      response.write("404 Not Found");
      response.end();
    });
  }
});

/*
spawnNethack(function(err, child) {
  if (err) throw err;
  child.stdout.pipe(process.stdout);
});
*/
