var addon = require('./build/Release/OPELRecording');
var cameraAPI = new addon.OPELRecording();
var path = '/home/ubuntu/'
require('date-utils');
var count=0;
var dt;
var d;

dt = new Date();
d = dt.toFormat('YYYY-MM-DD HH24:MI:SS');


cameraAPI.streamingStart("127.0.0.1", 5000);


var repeat = setInterval(function(){
		count++;
//			recObj.start(path+'['+d+']['+count+'].jpeg', 1, function(err, data){
//			console.log('count : '+ count);
	//	}); 
},	5000);
