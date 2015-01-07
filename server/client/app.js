var Socket = require('./socket');
var resource_loader = require('./resource_loader');

var caseCorrectUsername;
var currentLevel;
var tileWidth;
var tileHeight;
var tilesImageTileXCount;
var tilesImageTileYCount;

var tilesImage = resource_loader.fetchImage("tiles.png");
var tilesImageTileWidth = 128;
var tilesImageTileHeight = 128;

var NETHACK_MSG_TYPE_COUNT = 0;
var NETHACK_MSG_TYPE_GLYPH = NETHACK_MSG_TYPE_COUNT++;
var NETHACK_MSG_TYPE_SET_ALL_ROCK = NETHACK_MSG_TYPE_COUNT++;

var binaryMessageHandlers = [
  handleGlyph,
  handleSetAllRock,
];

var constants = require('../../build/constants');
var GLYPH_STONE = constants.GLYPH_CMAP_OFF + 0;

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

    handleSetAllRock();
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

function handleGlyph(dv) {
  var x = dv.getInt32(0, true);
  var y = dv.getInt32(4, true);
  var glyph = dv.getInt32(8, true);
  setGlyph(x, y, glyph);
}

function setGlyph(x, y, glyph) {
  currentLevel[y][x] = glyph;

  // not sure where this magic number comes from
  var tileIndex = glyph - 1515;

  var tileRow = Math.floor(tileIndex / tilesImageTileXCount);
  var tileCol = tileIndex % tilesImageTileXCount;

  context.drawImage(tilesImage, tileCol * tilesImageTileWidth, tileRow * tilesImageTileHeight,
      tilesImageTileWidth, tilesImageTileHeight,
      x * tileWidth, y * tileHeight,
      tileWidth, tileHeight);
}

function handleSetAllRock() {
  currentLevel = new Array(constants.ROWNO);
  for (var y = 0; y < constants.ROWNO; y += 1) {
    var col = new Array(constants.COLNO);
    currentLevel[y] = col;
    for (var x = 0; x < constants.COLNO; x += 1) {
      setGlyph(x, y, GLYPH_STONE);
    }
  }
}
