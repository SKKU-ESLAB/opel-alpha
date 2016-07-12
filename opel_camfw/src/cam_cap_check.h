#ifndef _CAM_CAP_CHECK_H_
#define _CAM_CAP_CHECK_H_
#include "cam_core.h"



class CamV4l2CapCheck{

				public:
			 	void checkDeviceCapabilities();
				CamV4l2CapCheck();
				void setCameraProperty(CameraProperty* camProp);
				
				private:
				CameraProperty* camProp;
				unsigned capabilities;
};










#endif /* _CAM_CAP_CHECK_H_ */
