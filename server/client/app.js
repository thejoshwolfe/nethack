var Socket = require('./socket');

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
var logOutBtn = document.getElementById('main-menu-logout');

loginOkBtn.addEventListener('click', loginOk);
loginRegisterBtn.addEventListener('click', showRegisterForm);
loginUsername.addEventListener('keydown', loginFormKeyDown);
loginPassword.addEventListener('keydown', loginFormKeyDown);

registerCancelBtn.addEventListener('click', showLoginForm);
registerOkBtn.addEventListener('click', registerOk);
registerUsername.addEventListener('keydown', registerFormKeyDown);
registerPassword.addEventListener('keydown', registerFormKeyDown);

logOutBtn.addEventListener('click', logOut);

var socket = new Socket();
socket.on('registerResult', onRegisterResult);
socket.on('loginResult', onLoginResult);


socket.on('connect', function() {
  showOnly(loginDiv);
  tryToLoginWithSavedCredentials();
});
socket.on('disconnect', function() {
  showOnly(loadingDiv);
});

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
  showOnly(mainMenuDiv);
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
  showOnly(mainMenuDiv);
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
  refreshPage();
}

function refreshPage() {
  location.href = location.protocol + "//" + location.host + "/";
}
