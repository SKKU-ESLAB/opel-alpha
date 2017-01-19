var opel_dir = process.env.OPEL_DIR;

var sysAppNIL = require(opel_dir + '/node/sysfw/nil');
var cameraNIL = require(opel_dir + '/node/OPELFaceDetection');

try {
  require('date-utils');
} catch (e) {
  console.log("\n[App-FaceDetection] Error occured");
  console.log("Please install 'date-utils' NPM module manually.");
  console.log(" $ cd " + opel_dir);
  console.log(" $ npm install date-utils\n");
  process.exit();
}

var count = 6;
var name, time;
var configPage = sysAppNIL.makeConfigPage();
configPage = sysAppNIL.addStrTextbox(configPage, "Name", "What is your name?", 10);

configPage = sysAppNIL.addTimeDialog(configPage, "Time", "What time do you want to wake up?", 1);

console.log(configPage);


sysAppNIL.sendConfigPage(configPage, function(configurableData){


	name = sysAppNIL.getData(configurableData, "Name");
	console.log("<GoodMorning>Set name : "+ name);

	time = sysAppNIL.getData(configurableData, "Time");
	console.log("<GoodMorning>Set time : "+ time);

	var repeat = setInterval(function(){
		var dt = new Date();
		var d = dt.toFormat('HH24:MI');

		if (time == d){
			console.log('<GoodMorning> : It`s time to wake up'); 
		
				var repeat3 = setInterval(function(){	
				cameraNIL.faceDetection(count, function(err, numberOfFace){
			

  		  			if(numberOfFace > 0){
						console.log('Face Detection !! ');
						console.log('Number of Face : ' + numberOfFace +'\n');
				        process.exit(1);
					}

			        else{
        				console.log('<GoodMorning> Show me your handsome face !!! \n');

			            var notiPage = sysAppNIL.makeEventPage("Time to wake up");
						notiPage = sysAppNIL.addEventText(notiPage, "Hey " + name);
						notiPage = sysAppNIL.addEventText(notiPage, "Wake up now");
						notiPage = sysAppNIL.addEventImg(notiPage, "/home/pi/morning.jpg");
						sysAppNIL.sendEventPageWithNoti(notiPage);

			        }
      			});

				}, 10000);
			
	
		}
		else{
			console.log('<GoodMorning> Waiting for the time [' + time + '] /// cur time [' + d +']');
		}

	}, 20000); //20sec

});

sysAppNIL.onTermination(function(){

		console.log('<onTermination> Called');
});


var repeat2 = setInterval(function(){
		}, 50000);
