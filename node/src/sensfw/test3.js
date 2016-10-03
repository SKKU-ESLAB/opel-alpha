console.log('Code start!');

var sensorManager = require("./build/Release/nil");



sensorManager.On("BUTTON", "PERIODIC", 1000, function(button){
		console.log("callback came");
		
		if(button.BUTTON > 2){
				console.log('get BUTTON !\n');
		}
		console.log("data: " + button.BUTTON);
})

setInterval(function(){},1000);

/*
sensorManager.On("SOUND", "PERIODIC", 1000, function(sound){
		console.log("callback came");
		if(sound.SOUND > 2){
				console.log('SOUND event!\n');
		}
		console.log("data: " + sound);
});
*/
