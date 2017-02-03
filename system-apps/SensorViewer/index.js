var opel_api_dir = process.env.OPEL_BIN_DIR + "/api/";
var commApi = require(opel_api_dir + "communication-api");
var sensorApi = require(opel_api_dir + "sensor-api");
var isViewReady = 0;

function mainCb(str, stat) {
	console.log("Incoming Message:" + str + "(" +  str.length + ")");

	if(str == "Connected") {
		isViewReady = 1;
  } else {
		isViewReady = 0;
  }
}

commApi.OpenChannel("Sensor Interface", mainCb);

var repeat = setInterval(function() {
		if(isViewReady == 1){
			var tch = sensorApi.Get("BUTTON");
			var acc = sensorApi.Get("ACC");
			var mot = sensorApi.Get("MOTION");
			var snd = sensorApi.Get("SOUND");
			var lit = sensorApi.Get("LIGHT");
			var vib = sensorApi.Get("VIBRATION");
			var tmp = sensorApi.Get("TEMP");

      var str = "{\"Touch\":\"" + tch.BUTTON
        + "\",\"Accelerometer\":\"" + acc.Z
        + "\",\"Motion\":\"" + mot.MOTION
        + "\",\"Sound\":\"" + snd.SOUND
        + "\",\"Light\":\"" + lit.LIGHT
        + "\",\"Vibration\":\"" + vib.VIBRATION
        + "\",\"Temperature\":\"" + tmp.TEMP
        + "\"}";
			commApi.SendMsg(str);
			console.log("Sent\n");
    }
}, 500);
