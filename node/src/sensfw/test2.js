console.log('Code start!');

var sensorManager = require("./build/Release/nil");

// Example code for getting sensor value 

/* When sensor values are more than one
console.log('the value X is ' + sensorManager.Get("ACC").X);
console.log('the value Y is ' + sensorManager.Get("ACC").Y);
console.log('the value Z is ' + sensorManager.Get("ACC").Z);
*/

/* When sensor value is one
console.log('The value is ' + sensorManager.Get("MOTION").MOTION);
*/


// Get Sensor List Example
var obj = sensorManager.GetSensorList();

console.log('The number of sensors is ' + obj.NUM);

for(var i=1; i<obj.NUM+1; i++){
  console.log(obj["SENSOR" + i.toString()]);
}

