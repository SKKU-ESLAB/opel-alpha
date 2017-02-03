var opel_api_dir = process.env.OPEL_BIN_DIR + "/api/";
var cameraApi = require(opel_api_dir + "camera-api");
var appApi = require(opel_api_dir + "app-api");

var streamObj = new cameraApi.OPELRecording();

//require('date-utils');
var count=0;
var dt;
var d;

//dt = new Date();
//d = dt.toFormat('YYYY-MM-DD HH24:MI:SS');

//two API Set 

/*******************************  API MODEL *******************************/
// Start API : count = N; Send Number of N Frames over network than callback callback 
// Start API : count = 0; Send Number of Infinte Frames over network unless stop API is invoked
/******************************** API SAMPLE *****************************/
// streamObj.start(int n, function(err, data); n != 0 : finite Number of Frame n == 0 : Infinite Number of Frame 
// streamObj.stop(); //aborting

streamObj.streamingStart("192.168.49.1", 5000);


nil.onTermination(function(){
			console.log('<onTermination Called');
			streamObj.streamingStop();
		});


/*
streamObj.start(0, function(error, data){
		if(data == 1)
			console.log('Streaming Done');		
		});

*/
var repeat = setInterval(function(){
}, 5000);
