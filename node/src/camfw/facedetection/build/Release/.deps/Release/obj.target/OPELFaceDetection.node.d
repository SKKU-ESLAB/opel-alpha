cmd_Release/obj.target/OPELFaceDetection.node := g++ -shared -pthread -rdynamic  -Wl,-soname=OPELFaceDetection.node -o Release/obj.target/OPELFaceDetection.node -Wl,--start-group Release/obj.target/OPELFaceDetection/src/addon.o Release/obj.target/OPELFaceDetection/src/OPELFaceDetection.o -Wl,--end-group -ldbus-1 -lglib-2.0 -L/usr/local/lib -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_hal -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_video -lopencv_videoio -lopencv_videostab