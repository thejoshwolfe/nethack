var Socket = require('./socket');

var loadingDiv = document.getElementById('loading');
var loginDiv = document.getElementById('login');
var loginOkBtn = document.getElementById('login-ok');
var loginRegisterBtn = document.getElementById('login-register');
var loginUsername = document.getElementById('login-username');
var loginPassword = document.getElementById('login-password');
var registerDiv = document.getElementById('register');
var registerOkBtn = document.getElementById('register-ok');
var registerCancelBtn = document.getElementById('register-cancel');
var registerUsername = document.getElementById('register-username');
var registerPassword = document.getElementById('register-password');
var registerErrDom = document.getElementById('register-err');
var mainMenuDiv = document.getElementById('main-menu');

loginRegisterBtn.addEventListener('click', showRegisterForm);
registerCancelBtn.addEventListener('click', showLoginForm);
registerOkBtn.addEventListener('click', registerOk);
registerUsername.addEventListener('keydown', registerFormKeyDown);
registerPassword.addEventListener('keydown', registerFormKeyDown);

var socket = new Socket();
socket.on('registerResult', onRegisterResult);


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
}

function showOnly(form) {
  hideAllForms();
  form.style.display = "";
}

function hideAllForms() {
  loadingDiv.style.display = "none";
  registerDiv.style.display = "none";
  loginDiv.style.display = "none";
}

function tryToLoginWithSavedCredentials() {
  var username = localStorage.getItem('username');
  var password = localStorage.getItem('password');
  tryLoginWithCredentials(username, password);
}

function registerOk(ev) {
  socket.send('register', {
    username: registerUsername.value,
    password: registerPassword.value,
  });
  showOnly(loadingDiv);
}

function tryLoginWithCredentials(username, password) {
  if (!username || !password) {
    showLoginForm();
    return;
  }
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
