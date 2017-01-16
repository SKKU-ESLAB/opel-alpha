var opel_dir = process.env.OPEL_DIR;

var addon = require(opel_dir +'/node/OPELFaceDetection');
var systemManager = require(opel_dir + '/node/sysfw/nil');

var count = 10;
function faceRecognition() {
addon.faceDetection(count, function(err, numberOfFace, imgPath){
			if(numberOfFace > 0){
			console.log('Face Detection !!! ');
			console.log('Number of Face : ' + numberOfFace +'\n');
			console.log('Image Path : ' + imgPath + '\n');
			systemManager.faceRecognitionWithNoti(imgPath);
			}
			else
			{
			console.log("Not Detection !!! \n");
			}		

});}

//faceRecognition();

var repeat = setInterval(faceRecognition, 8000);

systemManager.onTermination(function(){
		console.log("Cloud service app is killed");
		});
