var opel_dir = process.env.OPEL_DIR;
var comm = require(opel_dir + "/node/NIL_Comm");
var sensorManager = require(opel_dir + "/node/sensfw/nil");
var isViewReady = 0;

function mainCb(str, stat) {
	console.log("Incoming Message:" + str + "(" +  str.length + ")");

	if(str == "Connected") {
		isViewReady = 1;
  } else {
		isViewReady = 0;
  }
}

comm.OpenChannel("Sensor Interface", mainCb);

var repeat = setInterval(function() {
		if(isViewReady == 1){
			var tch = sensorManager.Get("BUTTON");
			var acc = sensorManager.Get("ACC");
			var mot = sensorManager.Get("MOTION");
			var snd = sensorManager.Get("SOUND");
			var lit = sensorManager.Get("LIGHT");
			var vib = sensorManager.Get("VIBRATION");
			var tmp = sensorManager.Get("TEMP");

      var str = "{\"Touch\":\"" + tch.BUTTON
        + "\",\"Accelerometer\":\"" + acc.Z
        + "\",\"Motion\":\"" + mot.MOTION
        + "\",\"Sound\":\"" + snd.SOUND
        + "\",\"Light\":\"" + lit.LIGHT
        + "\",\"Vibration\":\"" + vib.VIBRATION
        + "\",\"Temperature\":\"" + tmp.TEMP
        + "\"}";
			comm.SendMsg(str);
			console.log("Sent\n");
    }
}, 500);
