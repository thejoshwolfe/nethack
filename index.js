var spawn = require('child_process').spawn;
var fs = require('fs');
var mkdirp = require('mkdirp');
var Pend = require('pend');

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

spawnNethack(function(err, child) {
  if (err) throw err;
  child.stdout.pipe(process.stdout);
});
