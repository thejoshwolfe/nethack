var spawn = require('child_process').spawn;
var fs = require('fs');
var mkdirp = require('mkdirp');
var Pend = require('pend');
var createGzipStatic = require('connect-static');
var path = require('path');
var http = require('http');
var yawl = require('yawl');
var leveldown = require('leveldown');
var dbIterate = require('./db_iterate');
var uuid = require('./uuid');
var WritableStream = require('stream').Writable;

var httpPort = process.env.NETHACK_HTTP_PORT || 21119;
var httpHost = process.env.NETHACK_HTTP_HOST || '0.0.0.0';
var dbFilePath = process.env.NETHACK_DB_PATH || 'nethack.db';

var USERS_KEY_PREFIX = "Users.Username.";

var NETHACK_DIR = path.join(__dirname, '../../');

var validateUsernameRegex = /^[a-zA-Z0-9_~]*$/;
var MIN_USERNAME_LENGTH = 1;
var MAX_USERNAME_LENGTH = 32;
var MIN_PASSWORD_LENGTH = 1;
var MAX_PASSWORD_LENGTH = 1024;

var db = leveldown(dbFilePath);
var httpServer;

var users = {};

boot();

function Session(ws) {
  this.ws = ws;
  this.user = null;
  this.netHackProcess = null;
}

Session.prototype.end = function() {
  this.killChildProcess();
};

Session.prototype.send = function(name, args) {
  this.ws.sendText(JSON.stringify({name: name, args: args}));
};

Session.prototype.login = function(args) {
  var username = args.username;
  var password = args.password;

  var id = usernameToId(username);
  var existingUser = users[id];
  if (!existingUser) {
    this.send('loginResult', {err: 'invalid username'});
    return;
  }
  if (existingUser.password !== password) {
    this.send('loginResult', {err: 'invalid password'});
    return;
  }
  this.user = existingUser;
  this.send('loginResult', {username: this.user.username});
};

Session.prototype.register = function(args) {
  var username = args.username;
  var password = args.password;
  if (username.length > MAX_USERNAME_LENGTH || username.length < MIN_USERNAME_LENGTH) {
    this.send('registerResult', {err: 'username length must be between ' +
      MIN_USERNAME_LENGTH + ' and ' + MAX_USERNAME_LENGTH});
    return;
  }
  if (!validateUsernameRegex.test(username)) {
    this.send('registerResult', {err: 'invalid characters in username'});
    return;
  }
  if (password.length > MAX_PASSWORD_LENGTH || password.length < MIN_PASSWORD_LENGTH) {
    this.send('registerResult', {err: 'password length must be between ' +
      MIN_PASSWORD_LENGTH + ' and ' + MAX_PASSWORD_LENGTH});
    return;
  }
  var id = usernameToId(username);
  var existingUser = users[id];
  if (existingUser) {
    this.send('registerResult', {err: 'username already exists'});
    return;
  }
  this.user = createNewUser(username, password);
  this.send('registerResult', {username: this.user.username});
};

Session.prototype.play = function(args) {
  var self = this;
  if (!self.user) return;
  self.killChildProcess();
  self.netHackProcess = spawnNethack();
  self.netHackProcess.on('error', function(err) {
    self.send('playError', {err: 'Unable to spawn nethack process'});
    console.error("Unable to spawn nethack process:", err.stack);
    self.killChildProcess();
  });
  self.closeHook = function(returnCode) {
    self.send('playError', {err: 'NetHack process crashed without warning'});
    console.error(new Error("NetHack process crashed without warning").stack);
    self.netHackProcess = null;
  };
  self.netHackProcess.on('close', self.closeHook);
  var st = new WritableStream();
  var buf = new Buffer(0);
  st._write = function(buffer, encoding, callback) {
    buf = Buffer.concat([buf, buffer]);
    for (;;) {
      if (buf.length < 8) break;
      var msgType = buf.readUInt32LE(0);
      var msgSize = buf.readUInt32LE(4);
      if (buf.length - 8 < msgSize) break;
      var slice = buf.slice(0, 8 + msgSize);
      buf = buf.slice(8 + msgSize);
      self.ws.sendBinary(slice);
    }
    callback();
  };
  self.netHackProcess.stdout.pipe(st);
};

Session.prototype.killChildProcess = function() {
  if (this.netHackProcess) {
    this.netHackProcess.removeListener('close', this.closeHook);
    this.netHackProcess.kill();
    this.netHackProcess = null;
    this.closeHook = null;
  }
};

var messageHandlers = {
  register: Session.prototype.register,
  login: Session.prototype.login,
  play: Session.prototype.play,
};

function spawnNethack(cb) {
  var options = {
    stdio: ['pipe', 'pipe', process.stderr],
    cwd: NETHACK_DIR,
  };
  var exePath = path.join(NETHACK_DIR, 'build', 'nethack');
  var child = spawn(exePath, [], options);
  return child;
}

function boot() {
  var pend = new Pend();
  db.open(pend.hold());
  pend.go(createHttpServer);
  pend.wait(function(err) {
    if (err) throw err;
    cacheAllDb(function(err) {
      if (err) throw err;
      httpServer.listen(httpPort, httpHost, function() {
        console.info("HTTP server listening at http://" + httpHost + ":" + httpPort + "/");
      });
    });
  });
}

function cacheAllDb(cb) {
  dbIterate(db, USERS_KEY_PREFIX, processOne, function(err) {
    if (err) return cb(err);
    cb();
  });
  function processOne(key, value) {
    var user = deserializeUser(value);
    users[usernameToId(user.username)] = user;
  }
}

function saveUserCmd(user, dbCmdList) {
  dbCmdList.push({type: 'put', key: userKey(user), value: serializeUser(user)});
}

function saveUser(user) {
  var dbCmdList = [];
  saveUserCmd(user, dbCmdList);
  db.batch(dbCmdList, logIfDbError);
}

function userKey(user) {
  return USERS_KEY_PREFIX + usernameToId(user.username);
}

function createHttpServer(cb) {
  createGzipStatic({dir: path.join(__dirname, "../public")}, function(err, staticMiddleware) {
    if (err) return cb(err);
    httpServer = http.createServer(handleResponse);
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
      var session = new Session(ws);
      var pingInterval = setInterval(sendPing, 1000);
      ws.on('textMessage', function(msg) {
        var json;
        try {
          json = JSON.parse(msg);
        } catch (err) {
          console.error("received invalid JSON from web socket:", err.message);
          ws.close();
          return;
        }
        var handler = messageHandlers[json.name];
        if (!handler) {
          console.error("Invalid message name: " + json.name);
          return;
        }
        handler.call(session, json.args);
      });
      ws.on('error', function(err) {
        console.error("web socket client error:", err.stack);
      });
      ws.on('close', function() {
        clearInterval(pingInterval);
        session.end();
      });
      function sendPing() {
        ws.sendPingText("");
      }
    });
    cb();

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
}

function serializeUser(user) {
  return JSON.stringify(user);
}

function deserializeUser(payload) {
  return JSON.parse(payload);
}

function createNewUser(username, password) {
  var user = {
    username: username,
    password: password,
    createdAt: new Date().getTime(),
  };
  var id = usernameToId(username);
  users[id] = user;
  saveUser(user);
  return user;
}

function usernameToId(username) {
  return username.toLowerCase();
}

function logIfDbError(err) {
  if (err) {
    console.error("DB Error:", err.stack);
  }
}
