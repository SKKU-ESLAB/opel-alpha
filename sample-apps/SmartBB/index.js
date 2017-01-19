var opel_dir = process.env.OPEL_DIR;

var OPELCameraAddon = require(opel_dir +'/node/OPELRecording');
var faceDetectAddon = require(opel_dir +'/node/OPELFaceDetection');
var sensorAddon = require(opel_dir +'/node/sensfw/nil');
var systemManager = require(opel_dir +'/node/sysfw/nil');

var recObj = new OPELCameraAddon.OPELRecording();
var count = 10000;

var opel_dir = process.env.OPEL_DIR;
var opel_data_dir = process.env.OPEL_DIR + "/data/";
faceDetectAddon.faceDetection(count, function(err, numberOfFace) {
  if(numberOfFace > 0) {
    // Face Detection Event Handling
    var filePath = opel_data_dir + "BB[12345].mp4";
    var playSeconds = 10;
    var handler = function(err) {
      console.log("Face Detection Event: " + filePath);

      var notiPage = systemManager.makeEventPage("Blackbox video was recorded");
      notiPage = systemManager.addEventText(notiPage, 
          numberOfFace + " faces are detected.");
      notiPage = systemManager.addEventText(notiPage, 
          "File path: " + filePath);
      systemManager.sendEventPageWithNoti(notiPage);
    }

    console.log("Start recording...");
    recObj.RecordingStart(filePath, playSeconds, handler);
  }
});
