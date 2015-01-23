var Socket = require('./socket');
var resource_loader = require('./resource_loader');

var caseCorrectUsername;
var currentMap;
var traps = [];
var engravings = [];
var tileWidth;
var tileHeight;
var tilesImageTileXCount;
var tilesImageTileYCount;

// use arrows to adjust the tileset
var shiftAllTiles = 0;

var tilesImage = resource_loader.fetchImage("tiles.png");
var tilesImageTileWidth = 32;
var tilesImageTileHeight = 32;
var tilesImageWarningOffset = 1007;
var tilesImageSwallowOffset = 904;
var tilesImageZapOffset = 975;
var tilesImageExplodeOffset = 912;
var tilesImageMapOffset = 829;
var tilesImageTrapOffset = 870;
var tilesImageObjectOffset = 394;
var tilesImageMonsterOffset = 0;

var NETHACK_MSG_TYPE_COUNT = 0;
var NETHACK_MSG_TYPE_GLYPH = NETHACK_MSG_TYPE_COUNT++;
var NETHACK_MSG_TYPE_SET_ALL_ROCK = NETHACK_MSG_TYPE_COUNT++;

var binaryMessageHandlers = [
  handleMap,
  handleTraps,
  handleEngravings,
];

var constants = require('../../build/constants');
var nethack_h = require('../../build/nethack_h');

var loadingDiv = document.getElementById('loading');
var loginDiv = document.getElementById('login');
var loginOkBtn = document.getElementById('login-ok');
var loginRegisterBtn = document.getElementById('login-register');
var loginUsername = document.getElementById('login-username');
var loginPassword = document.getElementById('login-password');
var loginErrDom = document.getElementById('login-err');

var registerDiv = document.getElementById('register');
var registerOkBtn = document.getElementById('register-ok');
var registerCancelBtn = document.getElementById('register-cancel');
var registerUsername = document.getElementById('register-username');
var registerPassword = document.getElementById('register-password');
var registerErrDom = document.getElementById('register-err');

var mainMenuDiv = document.getElementById('main-menu');
var mainMenuUsername = document.getElementById('main-menu-username');
var logOutBtn = document.getElementById('main-menu-logout');
var playBtn = document.getElementById('main-menu-play');

var playDiv = document.getElementById('play');
var canvas = document.getElementById('canvas');
var playErrDom = document.getElementById('play-err');

var context = canvas.getContext('2d');

loginOkBtn.addEventListener('click', loginOk);
loginRegisterBtn.addEventListener('click', showRegisterForm);
loginUsername.addEventListener('keydown', loginFormKeyDown);
loginPassword.addEventListener('keydown', loginFormKeyDown);

registerCancelBtn.addEventListener('click', showLoginForm);
registerOkBtn.addEventListener('click', registerOk);
registerUsername.addEventListener('keydown', registerFormKeyDown);
registerPassword.addEventListener('keydown', registerFormKeyDown);

logOutBtn.addEventListener('click', logOut);
playBtn.addEventListener('click', playNetHack);

window.addEventListener('keydown', onKeyDown);
canvas.addEventListener('mousedown', onCanvasMouseDown);

var socket = new Socket();
socket.on('registerResult', onRegisterResult);
socket.on('loginResult', onLoginResult);
socket.on('playError', onPlayError);
socket.on('binaryMessage', onBinaryMessage);

socket.on('connect', function() {
  tryToLoginWithSavedCredentials();
});
socket.on('disconnect', function() {
  showOnly(loadingDiv);
  resetState();
});

function resetState() {
  caseCorrectUsername = null;
  currentMap = null;
  traps = [];
  engravings = [];
  tileWidth = null;
  tileHeight = null;
  tilesImageTileXCount = null;
  tilesImageTileYCount = null;
}

function showRegisterForm() {
  showOnly(registerDiv);
  registerUsername.value = "";
  registerPassword.value = "";
  registerUsername.focus();
  showRegisterErr(null);
}

function showLoginForm() {
  showOnly(loginDiv);
  loginUsername.value = "";
  loginPassword.value = "";
  loginUsername.focus();
  showLoginErr(null);
}

function showOnly(form) {
  hideAllForms();
  form.style.display = "";
}

function hideAllForms() {
  loadingDiv.style.display = "none";
  registerDiv.style.display = "none";
  loginDiv.style.display = "none";
  mainMenuDiv.style.display = "none";
  playDiv.style.display = "none";
}

function tryToLoginWithSavedCredentials() {
  var username = localStorage.getItem('username');
  var password = localStorage.getItem('password');
  if (!username || !password) {
    showLoginForm();
    return;
  }
  tryLoginWithCredentials(username, password);
}

function registerOk(ev) {
  socket.send('register', {
    username: registerUsername.value,
    password: registerPassword.value,
  });
  saveCredentials(registerUsername.value, registerPassword.value);
  showOnly(loadingDiv);
}

function tryLoginWithCredentials(username, password) {
  socket.send('login', {
    username: username,
    password: password,
  });
  showOnly(loadingDiv);
}

function registerFormKeyDown(ev) {
  if (ev.which === 27) {
    showLoginForm();
  } else if (ev.which === 13) {
    registerOk();
  }
}

function onRegisterResult(msg) {
  if (msg.err) {
    clearSavedCredentials();
    showRegisterErr(msg.err);
    showOnly(registerDiv);
    registerUsername.focus();
    registerUsername.select();
    return;
  }
  caseCorrectUsername = msg.username;
  showMainMenu();
}

function showMainMenu() {
  showOnly(mainMenuDiv);
  mainMenuUsername.textContent = caseCorrectUsername;
}

function showRegisterErr(errText) {
  if (!errText) {
    registerErrDom.style.display = "none";
    return;
  }
  registerErrDom.style.display = "";
  registerErrDom.textContent = errText;
}

function loginFormKeyDown(ev) {
  if (ev.which === 27) {
    showLoginForm();
  } else if (ev.which === 13) {
    loginOk();
  }
}

function loginOk() {
  var username = loginUsername.value;
  var password = loginPassword.value;
  saveCredentials(loginUsername.value, loginPassword.value);
  tryLoginWithCredentials(username, password);
}

function onLoginResult(msg) {
  if (msg.err) {
    clearSavedCredentials();
    showLoginErr(msg.err);
    showOnly(loginDiv);
    loginUsername.focus();
    loginUsername.select();
    return;
  }
  caseCorrectUsername = msg.username;
  showMainMenu();
}

function showLoginErr(errText) {
  if (!errText) {
    loginErrDom.style.display = "none";
    return;
  }
  loginErrDom.style.display = "";
  loginErrDom.textContent = errText;
}

function clearSavedCredentials() {
  localStorage.removeItem('username');
  localStorage.removeItem('password');
}

function saveCredentials(username, password) {
  localStorage.setItem('username', username);
  localStorage.setItem('password', password);
}

function logOut() {
  clearSavedCredentials();
  resetState();
  showLoginForm();
}

function playNetHack() {
  showOnly(loadingDiv);
  resource_loader.wait(function() {
    showOnly(playDiv);
    socket.send('play');

    canvas.width = 1800;
    canvas.height = 600;
    tileWidth = Math.floor(canvas.width / constants.COLNO);
    tileHeight = Math.floor(canvas.height / constants.ROWNO);
    // make them even
    tileWidth -= tileWidth % 2;
    tileHeight -= tileHeight % 2;

    tilesImageTileXCount = Math.floor(tilesImage.width / tilesImageTileWidth);
    tilesImageTileYCount = Math.floor(tilesImage.height / tilesImageTileHeight);

    canvas.focus();
  });
}

function showPlayErr(errText) {
  if (!errText) {
    playErrDom.style.display = "none";
    return;
  }
  playErrDom.style.display = "";
  playErrDom.textContent = errText;
}

function onPlayError(msg) {
  showPlayErr(msg.err);
}

function onBinaryMessage(buffer) {
  var dv = new DataView(buffer);
  var id = dv.getUint32(0, true);
  var handler = binaryMessageHandlers[id];
  if (!handler) throw new Error("no handler for id " + id);
  handler(new DataView(buffer, 8));
}

function dungeonFeatureToTileIndex(dungeonFeature) {
  switch (dungeonFeature) {
    case nethack_h.DungeonFeature.UNKNOWN:                   return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.STONE_WALL:                return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_LT:              return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_TR:              return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_RB:              return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_LB:              return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_LR:              return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_TB:              return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_LTR:             return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_TRB:             return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_LRB:             return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_LTB:             return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.ROOM_WALL_LTRB:            return tilesImageMapOffset + 0;
    case nethack_h.DungeonFeature.DOOR_CLOSED_H:             return tilesImageMapOffset + 15;
    case nethack_h.DungeonFeature.DOOR_CLOSED_V:             return tilesImageMapOffset + 15;
    case nethack_h.DungeonFeature.DOOR_OPEN_H:               return tilesImageMapOffset + 14;
    case nethack_h.DungeonFeature.DOOR_OPEN_V:               return tilesImageMapOffset + 14;
    case nethack_h.DungeonFeature.DOOR_BROKEN:               return tilesImageMapOffset + 13;
    case nethack_h.DungeonFeature.DOORWAY:                   return tilesImageMapOffset + 19;
    case nethack_h.DungeonFeature.ROOM_FLOOR:                return tilesImageMapOffset + 19;
    case nethack_h.DungeonFeature.CORRIDOR:                  return tilesImageMapOffset + 20;
    case nethack_h.DungeonFeature.STAIRS_UP:                 return tilesImageMapOffset + 22;
    case nethack_h.DungeonFeature.STAIRS_DOWN:               return tilesImageMapOffset + 23;
    case nethack_h.DungeonFeature.LADDER_UP:                 return tilesImageMapOffset + 24;
    case nethack_h.DungeonFeature.LADDER_DOWN:               return tilesImageMapOffset + 25;
    case nethack_h.DungeonFeature.ALTER:                     return tilesImageMapOffset + 26;
    case nethack_h.DungeonFeature.GRAVE:                     return tilesImageMapOffset + 27;
    case nethack_h.DungeonFeature.THRONE:                    return tilesImageMapOffset + 28;
    case nethack_h.DungeonFeature.SINK:                      return tilesImageMapOffset + 29;
    case nethack_h.DungeonFeature.FOUNTAIN:                  return tilesImageMapOffset + 30;
    case nethack_h.DungeonFeature.POOL:                      return tilesImageMapOffset + 31;
    case nethack_h.DungeonFeature.MOAT:                      return tilesImageMapOffset + 31;
    case nethack_h.DungeonFeature.ICE:                       return tilesImageMapOffset + 32;
    case nethack_h.DungeonFeature.LAVA:                      return tilesImageMapOffset + 33;
    case nethack_h.DungeonFeature.IRON_BARS:                 return tilesImageMapOffset + 17;
    case nethack_h.DungeonFeature.TREE:                      return tilesImageMapOffset + 18;
    case nethack_h.DungeonFeature.DRAWBRIDGE_RAISED:         return tilesImageMapOffset + 37;
    case nethack_h.DungeonFeature.DRAWBRIDGE_LOWERED:        return tilesImageMapOffset + 35;
    case nethack_h.DungeonFeature.PORTCULLIS_OPEN:           return tilesImageMapOffset + 19;
    case nethack_h.DungeonFeature.PLANE_OF_AIR:              return tilesImageMapOffset + 38;
    case nethack_h.DungeonFeature.PLANE_OF_AIR_CLOUD:        return tilesImageMapOffset + 39;
    case nethack_h.DungeonFeature.PLANE_OF_WATER:            return tilesImageMapOffset + 40;
    case nethack_h.DungeonFeature.PLANE_OF_WATER_AIR_BUBBLE: return tilesImageMapOffset + 38;
  }
  throw new Error("error code 333d543c6c6a9977:", dungeonFeature);
}

function trapTypeToTileIndex(trapType) {
  return tilesImageTrapOffset + trapType;
}

function renderEverything() {
  for (var y = 0; y < constants.ROWNO; y += 1) {
    for (var x = 0; x < constants.COLNO; x += 1) {
      var dungeonFeature = currentMap[y][x];
      renderTile(x, y, dungeonFeatureToTileIndex(dungeonFeature) + shiftAllTiles);
    }
  }
  traps.forEach(function(trap) {
    var coord = trap.location.coord;
    renderTile(coord.x, coord.y, trapTypeToTileIndex(trap.trapType) + shiftAllTiles);
  });
  // TODO: engravings?
}
function renderTile(x, y, tileIndex) {
  var tileRow = Math.floor(tileIndex / tilesImageTileXCount);
  var tileCol = tileIndex % tilesImageTileXCount;

  context.fillStyle = '#000000';
  context.fillRect(x * tileWidth, y * tileHeight, tileWidth, tileHeight);

  context.drawImage(tilesImage, tileCol * tilesImageTileWidth, tileRow * tilesImageTileHeight,
      tilesImageTileWidth, tilesImageTileHeight,
      x * tileWidth, y * tileHeight,
      tileWidth, tileHeight);
}

function handleMap(dv) {
  var reader = new DataViewReader(dv);
  currentMap = new Array(constants.ROWNO);
  for (var y = 0; y < constants.ROWNO; y += 1) {
    var col = new Array(constants.COLNO);
    currentMap[y] = col;
    for (var x = 0; x < constants.COLNO; x += 1) {
      var dungeonFeature = reader.readUint32();
      currentMap[y][x] = dungeonFeature;
    }
  }
  renderEverything();
}
function handleTraps(dv) {
  var reader = new DataViewReader(dv);
  traps = [];
  var trap_count = reader.readUint32();
  for (var i = 0; i < trap_count; i++) {
    traps.push(reader.readTrap());
  }
  renderEverything();
}
function handleEngravings(dv) {
  var reader = new DataViewReader(dv);
  engravings = [];
  var count = reader.readUint32();
  for (var i = 0; i < count; i++) {
    engravings.push(reader.readEngraving());
  }
  // render?
}

var keyDownHandlers = {
  // numpad 1
  97: function() {
    moveDir(-1, 1);
  },
  // numpad 2
  98: function() {
    moveDir(0, 1);
  },
  // numpad 3
  99: function() {
    moveDir(1, 1);
  },
  // numpad 4
  100: function() {
    moveDir(-1, 0);
  },
  // numpad 5
  101: function() {
    moveDir(0, 0);
  },
  // numpad 6
  102: function() {
    moveDir(1, 0);
  },
  // numpad 7
  103: function() {
    moveDir(-1, -1);
  },
  // numpad 8
  104: function() {
    moveDir(0, -1);
  },
  // numpad 9
  105: function() {
    moveDir(1, -1);
  },
  // testing:
  // left
  37: function() {
    adjustTileShift(-1);
  },
  // up
  38: function() {
    adjustTileShift(-tilesImageTileXCount);
  },
  // right
  39: function() {
    adjustTileShift(1);
  },
  // down
  40: function() {
    adjustTileShift(tilesImageTileXCount);
  },
};
function adjustTileShift(delta) {
  shiftAllTiles += delta;
  console.log("shiftAllTiles:", shiftAllTiles);
  renderEverything();
}

function onKeyDown(ev) {
  if (ev.ctrlKey || ev.altKey) return;
  // allow F1-F12
  if (112 <= ev.which && ev.which < 124) return;
  ev.preventDefault();
  ev.stopPropagation();

  var fn = keyDownHandlers[ev.which];
  if (fn) {
    fn(ev);
  } else {
    console.log("onKeyDown(" + ev.which + ", shift=" + ev.shiftKey + ")");
  }
}

function onCanvasMouseDown(ev) {
  var tileX = Math.floor(ev.clientX / tileWidth);
  var tileY = Math.floor(ev.clientY / tileHeight);
  var dungeonFeature = currentMap[tileY][tileX];
  console.log("x", tileX, "y", tileY, dungeonFeature);
}

function moveDir(x, y) {
  socket.send('move', {x: x, y: y});
}

function DataViewReader(dv) {
  this.dv = dv;
  this.cursor = 0;
}
DataViewReader.prototype.readEngraving = function () {
  return {
    location: this.readLocation(),
    engravingType: this.readUint32(),
    text: this.readString(),
  };
};
DataViewReader.prototype.readTrap = function() {
  return {
    location: this.readLocation(),
    trapType: this.readUint32(),
  };
};
DataViewReader.prototype.readLocation = function() {
  return {
    levelId: this.readUint526(),
    coord: this.readCoord(),
  };
};
DataViewReader.prototype.readUint526 = function() {
  var result = "";
  for (var i = 0; i < 8; i++) {
    result += this.readUint32().toString(16);
  }
  return result;
};
DataViewReader.prototype.readCoord = function() {
  return {
    x: this.readInt32(),
    y: this.readInt32(),
  };
};
DataViewReader.prototype.readInt32 = function() {
  var value = this.dv.getInt32(this.cursor, true);
  this.cursor += 4;
  return value;
};
DataViewReader.prototype.readUint32 = function() {
  var value = this.dv.getUint32(this.cursor, true);
  this.cursor += 4;
  return value;
};
DataViewReader.prototype.readString = function() {
  var byte_count = this.readUint32();
  var slice = new DataView(this.dv.buffer, this.dv.byteOffset + this.cursor, byte_count);
  var textDecoder = new TextDecoder("utf8", {fatal:true});
  var text = textDecoder.decode(slice);
  this.cursor += byte_count;
  return text;
};
