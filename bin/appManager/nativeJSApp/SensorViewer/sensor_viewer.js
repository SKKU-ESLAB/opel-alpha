var comm = require('./nil_comm/NIL_Comm');
var OPELFaceDetection = require('./nil_camera/OPELFaceDetection');
var sensorManager = require("./nil_sensor/nil");
var viewOn = 0;

function faceCb(err, num) {
	if(num > 0){
		console.log("Face :" + num + '\n');
		comm.SendMsg("Faces: " + num);
	}
	else{
		console.log("No faces detection\n");
		comm.SendMsg("No faces detection")
	}
}
function ackCb(str, stat) {
	console.log("Ack]Incoming Message:"+str+str.length)
}
function mainCb(str, stat) {
	console.log("Incoming Message:"+str+str.length)

	/*if(str == "hi"){
		console.log("OK?")
		comm.SendMsg("Hello Man")
	}
*/
	if(str == "Connected"){
		viewOn = 1;
	}
	else
		viewOn = 0;
//	else{
//		var strArray=str.split(':');
//		if(strArray[0].toLowerCase() == "get")
//		{
//			if(strArray[1].toLowerCase() == "temperature"){
//				var tmp = sensorManager.Get("TEMP");
//				comm.SendMsg("Temp:" + tmp.TEMP);
//			}
//			else if(strArray[1].toLowerCase() == "sound"){
//				var snd = sensorManager.Get("SOUND");
//				comm.SendMsg("Sound:" + snd.SOUND);
//			}
//			/*else if(strArray[1].toLowerCase() == "face"){
//				var cnt = 20;
//				OPELFaceDetection.faceDetection(cnt, faceCb)
//			}*/
//			else
//				comm.SendMsg("No sensor exists:"+strArray[1]);
//	
//			//Get Sensor Data as a string Value
//		}
//		else
//			comm.SendMsg("Correct Format = GET:[Sensor_Name]")
//	}

}

comm.OpenChannel('Sensor Intf', mainCb)

var repeat = setInterval(function(){
		if(viewOn == 1){
			var tch = sensorManager.Get("TOUCH");
			var acc = sensorManager.Get("ACC");
			var mot = sensorManager.Get("MOTION");
			var snd = sensorManager.Get("SOUND");
			var lit = sensorManager.Get("LIGHT");
			var vib = sensorManager.Get("VIBRATION");
			var tmp = sensorManager.Get("TEMP");
			
			var str = "{\"Touch\":\""+tch.TOUCH+"\",\"Accelerometer\":\""+acc.Z+"\",\"Motion\":\""+mot.MOTION+"\",\"Sound\":\""+snd.SOUND+"\",\"Light\":\""+lit.LIGHT+"\",\"Vibration\":\""+vib.VIBRATION+"\",\"Temperature\":\""+tmp.TEMP+"\"}";
			comm.SendMsg(str);
			console.log("Sent\n");

		}
		}, 500);

////sensorManager.On("MOTION", "NOTIFY", 5000, function() {
////	comm.SendMsg("Somebody there!")
////});
//
//sensorManager.On("SOUND", "NOTIFY", 100, function(){
//	var tmp = sensorManager.Get("SOUND");
//	if(tmp.SOUND > 10)
//		comm.SendMsg("Sound:" + tmp.SOUND);
//});
//
//sensorManager.On("VIBRATION", "NOTIFY", 200, function(){
//	comm.SendMsg("Vibration!")
//});
//
///* 아래 코드가 Touch 왔을 때 불리면 됩니다. */
////comm.SendMsg("Touch!")
