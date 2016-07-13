var addon = require('./build/Release/OPELRecording');
var recObj = new addon.OPELRecording();
var path = '/home/pi/camData/'
require('date-utils');
var count=0;
var dt;
var d;

dt = new Date();
d = dt.toFormat('YYYY-MM-DD HH24:MI:SS');

recObj.init(function(width, height, buffer_size){ 
		console.log('width : ', width);
		console.log('height : ', height);
		console.log('buffer_size : ', buffer_size);
		});

/*recObj.start(path+'['+d+'].jpeg', 1, function(err, data){
		console.log(path+'['+d+'].jpeg saved');
		});*/ 


recObj.start("/home/pi/camData/some.mjpg", 250, function(error, data){
		if(data == 1)
			console.log('/home/pi/camData/some.mjpg Saved\n');		
		});

/*recObj.start("/home/pi/camData/some1.mjpg", 300, function(error, data){
		if(data == 1)
			console.log('/home/pi/camData/some1.mjpg Saved\n');		
		});

recObj.start("/home/pi/camData/some2.mjpg", 300, function(error, data){
		if(data == 1)
			console.log('/home/pi/camData/some2.mjpg Saved\n');		
		});*/
/*recObj.start("/home/pi/camData/some1.mjpg", 200, function(error, data){
		if(data == 1)
			console.log('/home/pi/camData/some1.mjpg Saved\n');
		});*/

var repeat = setInterval(function(){
		count++;
//			recObj.start(path+'['+d+']['+count+'].jpeg', 1, function(err, data){
//			console.log('count : '+ count);
	//	}); 
},	5000);
