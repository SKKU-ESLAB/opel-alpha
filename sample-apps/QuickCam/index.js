console.log("Application start");

var opel_dir = process.env.OPEL_DIR;
var addon = require(opel_dir + '/node/OPELRecording');
var sensorManager = require(opel_dir + '/node/sensfw/nil');
var sysAppNIL = require(opel_dir + '/node/sysfw/nil');
var cameraAPI = new addon.OPELRecording();
var count = 0;

var prev;
var curr = 0;
var cnt=0;

console.log("setInterval start");
setInterval(function(){
		console.log("Inside setInterval");
		
		var touch = sensorManager.Get("BUTTON").BUTTON;
		curr = touch;
		console.log('TOUCH value: ' + touch);
		console.log('curr:'+curr+'/prev:'+prev+'/touch'+touch);
		if(prev != curr) {
      // Touch sensor event
			prev = curr;
			if(curr == 0) {
				console.log('Capture!');
				cnt++;
        var opel_dir = process.env.OPEL_DIR;
        var opel_data_dir = opel_dir + "/data/";
				var fileName = opel_data_dir + '/quickCam' + cnt + '.jpeg';
				cameraAPI.SnapshotStart(fileName);
				var notiPage = sysAppNIL.makeEventPage("Snapshot Saved");
				notiPage = sysAppNIL.addEventText(notiPage, "Quick Cam Snapshot");
				notiPage = sysAppNIL.addEventImg(notiPage, fileName);

        // Polling the sensor Data (Crap or Touch Event)
				sysAppNIL.sendEventPageWithNoti(notiPage);
			}
			else
				console.log("Touch:"+curr);
		}

},1000);
