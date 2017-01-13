console.log("Application start");

var opel_dir = process.env.OPEL_DIR;
var addon = require(opel_dir + '/node/OPELRecording');
var sensorManager = require(opel_dir + '/node/sensfw/nil');
var sysAppNIL = require(opel_dir + '/node/sysfw/nil');
var cameraAPI = new addon.OPELRecording();
var count = 0;

var dt// = new Date();
var d;//= dt.toFormat('YYYY-MM-DD HH24:MI:SS');

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
			prev = curr;
			if(curr == 0) {
				console.log('Capture!');
				d = cnt++;
				var fileName = '/home/ubuntu/opelData/quickCam'+d+'.jpeg';
				cameraAPI.SnapshotStart(fileName);
				var notiPage = sysAppNIL.makeEventPage("Snapshot Saved");
				notiPage = sysAppNIL.addEventText(notiPage, "Quick Cam Snapshot");
				notiPage = sysAppNIL.addEventImg(notiPage, fileName);
				sysAppNIL.sendEventPageWithNoti(notiPage);// Polling the sensor Data (Crap or Touch Event)
			}
			else
				console.log("Touch:"+curr);
		}

},1000);

//sysAppNIL.onTermination(function(){
//		console.log('<onTermination> Called');
//});
//
