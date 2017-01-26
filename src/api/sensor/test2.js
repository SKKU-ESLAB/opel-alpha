console.log('Code start!');

var sensorManager = require("./build/Release/nil");

setInterval(function(){
console.log('the value is ' + sensorManager.Get("BUTTON").BUTTON);
},10);
