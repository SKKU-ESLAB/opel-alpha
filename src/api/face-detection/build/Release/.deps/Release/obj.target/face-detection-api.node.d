cmd_Release/obj.target/face-detection-api.node := g++ -shared -pthread -rdynamic  -Wl,-soname=face-detection-api.node -o Release/obj.target/face-detection-api.node -Wl,--start-group Release/obj.target/face-detection-api/src/addon.o Release/obj.target/face-detection-api/src/OPELFaceDetection.o -Wl,--end-group -ldbus-1 -lglib-2.0 -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lopencv_hal
