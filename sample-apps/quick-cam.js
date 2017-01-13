var opel_dir = process.env.OPEL_DIR;

var addon = require(opel_dir + '/node/OPELRecording');
var sensorManager = require(opel_dir + '/node/sensfw/nil');
var sysAppNIL = require(opel_dir + '/node/sysfw/nil');
var recObj = new addon.OPELRecording();

var count = 0;

var dt// = new Date();
var d;//= dt.toFormat('YYYY-MM-DD HH24:MI:SS');

var prev;
var curr;
var cnt=0;

recObj.init(function(width, height, buffer_size){
			console.log('initialization Done\n');
		});

var repeat = setInterval(function(){
		var tmp = sensorManager.Get("TOUCH");

		curr = tmp.TOUCH;
		//console.log('curr:'+curr+'/prev:'+prev+'/touch'+tmp.TOUCH);
		if(prev != curr){
			prev = curr;
		if(curr == 0){
		console.log('Capture!');
			dt = new Date(); 
			d = cnt++;
			var fileName = opel_dir + '../opelData/quickCam'+d+'.jpeg';
			recObj.start(fileName, 1, function(err, data){
				if(data){
				console.log(opel_dir + "../opelData/quickCam.jpeg Saved\n");
				//Notify
				var notiPage = sysAppNIL.makeEventPage("Snapshot Saved");
				notiPage = sysAppNIL.addEventText(notiPage, "Quick Cam Snapshot");
				notiPage = sysAppNIL.addEventImg(notiPage, fileName);
				sysAppNIL.sendEventPageWithNoti(notiPage);// Polling the sensor Data (Crap or Touch Event)
				}
				});	
		}
		else{
			console.log("Touch:"+curr);
		}

		}
		}, 100);
/*sensorManager.On("TOUCH", "PERIODIC", 100,  function(tch){
		//curr = tch.TOUCH;
		//if(prev != curr){
		//	prev = curr;
		//if(curr == 0){
		console.log('Capture!');
		//	dt = new Date(); 
		//	d = dt.toFormat('HH24:MI:SS');
		//	var fileName = '/home/pi/camData/'+'['+d+'][snapshot].jpeg';
		//	recObj.start(fileName, 1, function(err, data){
		//		if(data){
		//		console.log("/home/pi/camData/"+"["+d+"].jpeg Saved\n");
		//		//Notify
		//		var notiPage = sysAppNIL.makeNotiPage("Snapshot Saved");
		//		notiPage = sysAppNIL.addNotiText(notiPage, "Quick Cam Snapshot");
		//		notiPage = sysAppNIL.addNotiImg(notiPage, "./../../../../../../.."+fileName);
		//		sysAppNIL.sendNotiPage(notiPage);// Polling the sensor Data (Crap or Touch Event)
		//		}
		//		});	
		//}
		//}
});
*/
//
sysAppNIL.onTermination(function(){

		console.log('<onTermination> Called');
});
//

