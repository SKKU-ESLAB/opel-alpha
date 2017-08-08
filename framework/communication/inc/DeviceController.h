#ifndef __DEVICE_CONTROLLER_H__
#define __DEVICE_CONTROLLER_H__

#include <vector>

class DeviceController {
  public:
    DeviceController()
      : mIsTurnedOn(false) { }

    virtual bool turnOn() {
      this->mIsTurnedOn = true;
      return 0;
    }

    virtual bool turnOff() {
      this->mIsTurnedOn = false;
      return 0;
    }

    bool isTurnedOn() { return this->mIsTurnedOn; }

  protected:
    bool mIsTurnedOn;
};

class BluetoothDeviceController : DeviceController {
  public:
    virtual bool turnOn();
    virtual bool turnOff();
  protected:
};

class WifiDirectDeviceController : DeviceController {
  public:
    virtual bool turnOn();
    virtual bool turnOff();
  protected:
};

#endif /* !defined(__DEVICE_CONTROLLER_H__) */
