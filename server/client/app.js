var Socket = require('./socket');
var resource_loader = require('./resource_loader');

var caseCorrectUsername;
var currentLevel;
var tileWidth;
var tileHeight;
var tilesImageTileXCount;
var tilesImageTileYCount;

var tilesImage = resource_loader.fetchImage("tiles.png");
var tilesImageTileWidth = 32;
var tilesImageTileHeight = 32;
var tilesImageWarningOffset = 1007;
var tilesImageSwallowOffset = 904;
var tilesImageZapOffset = 975;
var tilesImageExplodeOffset = 912;
var tilesImageMapOffset = 829;
var tilesImageObjectOffset = 394;
var tilesImageMonsterOffset = 0;

var NETHACK_MSG_TYPE_COUNT = 0;
var NETHACK_MSG_TYPE_GLYPH = NETHACK_MSG_TYPE_COUNT++;
var NETHACK_MSG_TYPE_SET_ALL_ROCK = NETHACK_MSG_TYPE_COUNT++;

var binaryMessageHandlers = [
  handleMap,
];

var constants = require('../../build/constants');
var GLYPH_STONE = constants.GLYPH_CMAP_OFF + 0;
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
  currentLevel = null;
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

function glyphTileIndex(glyph) {
  var offset;

  if ((offset = (glyph - constants.GLYPH_WARNING_OFF)) >= 0) {
    return tilesImageWarningOffset + offset;
  }
  if ((offset = (glyph - constants.GLYPH_SWALLOW_OFF)) >= 0) {
    return tilesImageSwallowOffset + (offset & 0x7);
  }
  if ((offset = (glyph - constants.GLYPH_ZAP_OFF)) >= 0) {
    return tilesImageZapOffset + (offset & 0x3);
  }
  if ((offset = (glyph - constants.GLYPH_EXPLODE_OFF)) >= 0) {
    return tilesImageExplodeOffset + (offset % 9);
  }
  if ((offset = (glyph - constants.GLYPH_CMAP_OFF)) >= 0) {
    return tilesImageMapOffset + offset;
  }
  if ((offset = (glyph - constants.GLYPH_OBJ_OFF)) >= 0) {
    return tilesImageObjectOffset + offset;
  }
  if ((offset = (glyph - constants.GLYPH_RIDDEN_OFF)) >= 0) {
    return tilesImageMonsterOffset + offset;
  }
  if ((offset = (glyph - constants.GLYPH_BODY_OFF)) >= 0) {
    // corpse. if we figure out how to do a special corpse per monster we could do that here
    return 640;
  }
  if ((offset = (glyph - constants.GLYPH_DETECT_OFF)) >= 0) {
    return tilesImageMonsterOffset + offset;
  }
  if ((offset = (glyph - constants.GLYPH_INVIS_OFF)) >= 0) {
    // invisible monster
    return 393;
  }
  if ((offset = (glyph - constants.GLYPH_PET_OFF)) >= 0) {
    return tilesImageMonsterOffset + offset;
  }

  return tilesImageMonsterOffset + glyph;
}

function setGlyph(x, y, glyph) {
  currentLevel[y][x] = glyph;

  // not sure where this magic number comes from
  var tileIndex = glyphTileIndex(glyph);

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
  currentLevel = new Array(constants.ROWNO);
  var i = 0;
  for (var y = 0; y < constants.ROWNO; y += 1) {
    var col = new Array(constants.COLNO);
    currentLevel[y] = col;
    for (var x = 0; x < constants.COLNO; x += 1) {
      var dungeon_feature = dv.getInt32(i, true);
      setGlyph(x, y, dungeon_feature);
      i += 4;
    }
  }
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
};

function onKeyDown(ev) {
  var fn = keyDownHandlers[ev.which];
  if (fn) {
    fn(ev);
    ev.preventDefault();
    ev.stopPropagation();
  }
}

function onCanvasMouseDown(ev) {
  var tileX = Math.floor(ev.clientX / tileWidth);
  var tileY = Math.floor(ev.clientY / tileHeight);
  var glyph = currentLevel[tileY][tileX];
  console.log("x", tileX, "y", tileY, glyph);
}

function moveDir(x, y) {
  socket.send('move', {x: x, y: y});
}
