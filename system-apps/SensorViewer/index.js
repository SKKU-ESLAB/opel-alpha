var opel_api_dir = process.env.OPEL_BIN_DIR + "/api/";
var appApi = require(opel_api_dir + "app-api");
var sensorApi = require(opel_api_dir + "sensor-api");
var NIL_MSG_TO_SENSOR_VIEWER = "1102";

var repeat = setInterval(function() {
  var tch = sensorApi.Get("BUTTON");
  var acc = sensorApi.Get("ACC");
  var mot = sensorApi.Get("MOTION");
  var snd = sensorApi.Get("SOUND");
  var lit = sensorApi.Get("LIGHT");
  var vib = sensorApi.Get("VIBRATION");
  var tmp = sensorApi.Get("TEMP");

  var str = "{\"type\":\"" + NIL_MSG_TO_SENSOR_VIEWER
    + "\"\"Touch\":\"" + tch.BUTTON
    + "\",\"Accelerometer\":\"" + acc.Z
    + "\",\"Motion\":\"" + mot.MOTION
    + "\",\"Sound\":\"" + snd.SOUND
    + "\",\"Light\":\"" + lit.LIGHT
    + "\",\"Vibration\":\"" + vib.VIBRATION
    + "\",\"Temperature\":\"" + tmp.TEMP
    + "\"}";
  appApi.sendMsgToSensorViewer(str);
  console.log("Sent\n");
}, 500);
