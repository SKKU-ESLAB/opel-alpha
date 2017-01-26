var addon = require('./build/Release/OPELRecording');
var cameraAPI = new addon.OPELRecording();
var opel_dir = process.env.OPEL_DIR;
require('date-utils');
var count=0;
var dt;
var d;

dt = new Date();
d = dt.toFormat('YYYY-MM-DD HH24:MI:SS');


cameraAPI.SnapshotStart(opel_dir+"/some.jpg");


var repeat = setInterval(function(){
		count++;
//			recObj.start(opel_dir+'['+d+']['+count+'].jpeg', 1, function(err, data){
//			console.log('count : '+ count);
	//	}); 
},	5000);
