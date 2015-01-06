var Pend = require('pend');
var pend = new Pend();

var total = 0;
var complete = 0;

exports.fetchImage = fetchImage;
exports.wait = wait;

function fetchImage(path) {
  total += 1;
  var img = new Image();
  img.src = path;
  var cb = pend.hold();
  img.onload = function() {
    complete += 1;
    cb();
  };
  return img;
}

function wait(cb) {
  pend.wait(cb);
}

function getProgressAmount() {
  return complete;
}

function getProgressTotal() {
  return total;
}
