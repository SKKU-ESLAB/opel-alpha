require('date-utils');
var OPELCameraAddon = require('../../../nil_camera/OPELRecording');
var OPELSensorAddon = require('../../../nil_sensor/nil');
var BlackBox = new OPELCameraAddon.OPELRecording();

var dt;
var d;
var motion;
var motionFlag = 1;
dt = new Date();
d = dt.toFormat('YYYY-MM-DD HH24:MI:SS');

var path = '/home/pi/BlackBox/';

BlackBox.init(function(width, height, buffer_size){
		console.log("BlackBox Setting");
		console.log('width : ', width);
		console.log('height : ', height);
		console.log('FPS : 30');
		});



var repeat = setInterval(function(){

		if(motionFlag)	
		{
		motion = OPELSensorAddon.Get('MOTION');
		if(motion)
		{
		console.log("Motion Detected!!!");
		motionFlag = 0;
		BlackBox.start(path+'[MotionDetected]['+d+'].mjpg', 300/*300frame: 10sec*/, function(err, data){
			if(data){
			console.log(path+'['+d+'].mjpeg Video Saved');
			motionFlag = 1;
			}
			});


		}

		}

}, 1000);
