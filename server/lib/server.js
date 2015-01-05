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

var httpPort = process.env.NETHACK_HTTP_PORT || 21119;
var httpHost = process.env.NETHACK_HTTP_HOST || '0.0.0.0';
var dbFilePath = process.env.NETHACK_DB_PATH || 'nethack.db';

var USERS_KEY_PREFIX = "Users.";

var validateUsernameRegex = /^[a-zA-Z0-9_~]*$/;
var MIN_USERNAME_LENGTH = 1;
var MAX_USERNAME_LENGTH = 32;
var MIN_PASSWORD_LENGTH = 1;
var MAX_PASSWORD_LENGTH = 1024;

var messageHandlers = {
  register: register,
};

var db = leveldown(dbFilePath);
var httpServer;

var users = {};

boot();

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
        console.info("TODO: users", users);
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
    users[user.id] = user;
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
  return USERS_KEY_PREFIX + user.id;
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
      console.info("ws client connected");
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
        handler(ws, json.args);
      });
      ws.on('error', function(err) {
        console.error("web socket client error:", err.stack);
      });
      ws.on('close', function() {
        console.info("ws client closed");
      });
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

/*
spawnNethack(function(err, child) {
  if (err) throw err;
  child.stdout.pipe(process.stdout);
});
*/

function register(ws, msg) {
  var username = msg.username;
  var password = msg.password;
  if (username.length > MAX_USERNAME_LENGTH || username.length < MIN_USERNAME_LENGTH) {
    send(ws, 'registerResult', {err: 'username length must be between ' +
      MIN_USERNAME_LENGTH + ' and ' + MAX_USERNAME_LENGTH});
    return;
  }
  if (!validateUsernameRegex.test(username)) {
    send(ws, 'registerResult', {err: 'invalid characters in username'});
    return;
  }
  if (password.length > MAX_PASSWORD_LENGTH || password.length < MIN_PASSWORD_LENGTH) {
    send(ws, 'registerResult', {err: 'password length must be between ' +
      MIN_PASSWORD_LENGTH + ' and ' + MAX_PASSWORD_LENGTH});
    return;
  }
  var id = usernameToId(username);
  var existingUser = users[id];
  if (existingUser) {
    send(ws, 'registerResult', {err: 'username already exists'});
    return;
  }
  createNewUser(username, password);
  send(ws, 'registerResult', {});
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
}

function send(ws, name, args) {
  ws.sendText(JSON.stringify({name: name, args: args}));
}

function usernameToId(username) {
  return username.toLowerCase();
}

function logIfDbError(err) {
  if (err) {
    console.error("DB Error:", err.stack);
  }
}
