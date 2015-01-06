var Socket = require('./socket');

var caseCorrectUsername;
var vision;
var tileWidth;
var tileHeight;

var roomFeatures = require('./room').roomFeatures;
var constants = require('../../build/constants');

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
socket.on('vision', onVision);

socket.on('connect', function() {
  tryToLoginWithSavedCredentials();
});
socket.on('disconnect', function() {
  showOnly(loadingDiv);
  resetState();
});

function resetState() {
  caseCorrectUsername = null;
  vision = null;
  tileWidth = null;
  tileHeight = null;
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
  showOnly(playDiv);
  socket.send('play');

  canvas.width = 1067;
  canvas.height = 600;
  tileWidth = Math.floor(canvas.width / constants.COLNO);
  tileHeight = Math.floor(canvas.height / constants.ROWNO);
  // make them even
  tileWidth -= tileWidth % 2;
  tileHeight -= tileHeight % 2;
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

function onVision(msg) {
  vision = msg;
  renderVision();
}

function renderVision() {
  // clear canvas to black
  context.fillStyle = '#000000'
  context.fillRect(0, 0, canvas.width, canvas.height);

  context.fillStyle = '#ffffff';
  context.textAlign = 'center';
  context.textBaseline = 'middle';
  context.font = '12px sans-serif';
  for (var y = 0; y < constants.ROWNO; y += 1) {
    for (var x = 0; x < constants.COLNO; x += 1) {
      var glyph = vision[y][x];
      var c = glyphToChar(glyph);
      context.fillText(c,
          x * tileWidth + tileWidth / 2,
          y * tileHeight + tileHeight / 2);
    }
  }
}

function glyphToChar(glyph) {
  var index = glyph - constants.GLYPH_CMAP_OFF;
  var c = roomFeatures[index];
  return c || '?';
}
