console.log('Code start!');

var sensorManager = require("./build/Release/nil");



sensorManager.On("BUTTON", "PERIODIC", 5000, function(button){
      console.log("callback came");
          
          if(button.BUTTON > 2){
                    console.log('get BUTTON !\n');
                        }
              console.log("data: " + button.BUTTON);
})

sensorManager.On("BUTTON", "PERIODIC", 2500, function(button){
      console.log("callback came");
          
          if(button.BUTTON > 2){
                    console.log('get BUTTON !\n');
                        }
              console.log("data: " + button.BUTTON);
})


sensorManager.On("BUTTON", "PERIODIC", 1000, function(button){
      console.log("callback came");
          
          if(button.BUTTON > 2){
                    console.log('get BUTTON !\n');
                        }
              console.log("data: " + button.BUTTON);
})
setInterval(function(){},1000);
