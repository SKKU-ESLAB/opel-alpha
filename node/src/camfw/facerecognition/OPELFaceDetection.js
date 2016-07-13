var addon = require('./build/Release/OPELFaceDetection');

var count = 10;


addon.faceRecognition(count, function(err, numberOfFace, imgPath){
		if(numberOfFace > 0){
			console.log('Face Detection !!! ');
			console.log('Number of Face : ' + numberOfFace +'\n');
			console.log('Image Path : ' + imgPath + '\n');
			//이부분에 noti 관련 api로 noti 알려
				
		}	
		else
		{
			console.log('Not Detection !!! \n');
		}
	});

