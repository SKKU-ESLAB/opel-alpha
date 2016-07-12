#ifndef _CAM_CTRL_H_
#define _CAM_CTRL_H_

#include "cam_core.h"

#define W_VGA 640
#define H_VGA 480

#define W_SVGA 800 
#define H_SVGA 600

#define W_XGA 1024
#define H_XGA 768

#define W_SXGA 1280
#define H_SXGA 1024

#define W_HD 1280
#define H_HD 720

#define W_FHD 1920
#define H_FHD 1080

class Camera_Ctrl
{
	public:
		static double getProperty(int) const;
		static double setProperty(int, double);
		static getInstance(Camera* cam);

	private: 
		Camera* cam;
};
















#endif /* _CAM_CTRL_H_ */
