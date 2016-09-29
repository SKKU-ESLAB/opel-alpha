console.log('Code start!');

var sensorManager = require("./build/Release/nil");

sensorManager.On("SOUND", "PERIODIC", 1000, function(sound){
		console.log("callback came");
		if(sound.SOUND > 2){
				console.log('SOUND event!\n');
		}
		console.log("data: " + sound);
});

