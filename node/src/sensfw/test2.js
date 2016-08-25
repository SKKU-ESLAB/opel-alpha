console.log('Code start!');


var sensorManager = require("./build/Release/nil");

console.log('the value is ' + sensorManager.Get("LIGHT").LIGHT);

