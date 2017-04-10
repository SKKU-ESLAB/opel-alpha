console.log("Application start");

var opel_api_dir = process.env.OPEL_BIN_DIR + "/api/";
var cameraApi = require(opel_api_dir + "camera-api");
var sensorApi = require(opel_api_dir + "sensor-api");
var appApi = require(opel_api_dir + "app-api");

var recordingObj = new cameraApi.OPELRecording();
var count = 0;

var prev;
var curr = 0;
var cnt=0;

console.log("setInterval start");
setInterval(function(){
		console.log("Inside setInterval");
		
		var touch = sensorApi.Get("BUTTON").BUTTON;
		curr = touch;
		console.log('TOUCH value: ' + touch);
		console.log('curr:'+curr+'/prev:'+prev+'/touch'+touch);
		if(prev != curr) {
      // Touch sensor event
			prev = curr;
			if(curr == 0) {
				console.log('Capture!');
				cnt++;
        var opel_data_dir = process.env.OPEL_DATA_DIR;
				var fileName = opel_data_dir + '/quickCam' + cnt + '.jpeg';
				recordingObj.snapshot(0, fileName);
				var notiPage = appApi.makeEventPage("Snapshot Saved");
				notiPage = appApi.addEventText(notiPage, "Quick Cam Snapshot");
				notiPage = appApi.addEventImg(notiPage, fileName);

        // Polling the sensor Data (Crap or Touch Event)
				appApi.sendEventPageWithNoti(notiPage);
			}
			else
				console.log("Touch:"+curr);
		}

},1000);
