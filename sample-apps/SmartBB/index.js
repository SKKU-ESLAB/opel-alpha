var opel_dir = process.env.OPEL_DIR;

var OPELCameraAddon = require(opel_dir +'/node/OPELRecording');
var faceDetectAddon = require(opel_dir +'/node/OPELFaceDetection');
var sensorAddon = require(opel_dir +'/node/sensfw/nil');
var systemManager = require(opel_dir +'/node/sysfw/nil');


var recObj = new OPELCameraAddon.OPELRecording();
var count = 10000;
var dt, d;
var cnt=0;

var opel_dir = process.env.OPEL_DIR;
var opel_data_dir = process.env.OPEL_DIR + "/data/";
var alreadyRunning = false;
var temp;

//var repeat = setInterval(function(){
		alreadyRunning = true;
		faceDetectAddon.faceDetection(count, function(err, numberOfFace){
				if(numberOfFace > 0)
				{
				
				console.log("FaceDetection");
				recObj.RecordingStart(opel_data_dir  + 'BB[12345].mp4', 10, function(err){
						var notiPage = systemManager.makeEventPage('Blackbox video was recorded');
						notiPage = systemManager.addEventText(notiPage, 'File name : '+ d + '.mp4');
						systemManager.sendEventPageWithNoti(notiPage);
						});
				}
				else
				alreadyRunning = false; 
				});
//}, 500);

/*systemManager.onTermination(function(){

		console.log('<onTermination> Called');
		});*/
