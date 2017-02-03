[![License](https://img.shields.io/badge/licence-Apache%202.0-brightgreen.svg?style=flat)](LICENSE)

# OPEL: Open Platform Event Logger (Alpha Version)
## Quick Start
### How to Get the Source Code
```
$ git clone https://github.com/sinban04/opel-alpha
$ cd opel-alpha
```

### How to Switch to Target Device's Branch
* Alpha 1(Raspberry Pi 2 or Raspberry Pi 3)
```
$ git checkout origin/raspberry-pi2_3
```
* Alpha 2(Nvidia TX1)
```
$ git checkout origin/tegraTX1
```
* Alpha 3
```
$ git checkout origin/alpha3
```

### How to Install Prerequisites
It is dependent on target device.

In example of tegraTX1:

```
$ ./target/tegraTX1/install-deps-tegraTX1.sh
```

### How to Build (Alpha 2, 3)
```
$ cmake .
$ make
```

For more details, see [Getting Started](https://github.com/sinban04/opel-alpha/wiki/Getting-Started).

### How to Install (Alpha 3)
In example of tegraTX1:

```
$ sudo ./scripts/install.sh --target=tegraTX1
```

You need target profile on ```target/TARGET_NAME/profile.env``` before running install script.

## Documentation
* [Getting Started](https://github.com/sinban04/opel-alpha/wiki/Getting-Started)

## Videos
* [OPEL Demo Video: Raspberry Pi 2](https://www.youtube.com/watch?v=6iI4zDDX-YE) (2016.01.14)
* [OPEL Demo Scenario Video: Smart Guardner](https://www.youtube.com/watch?v=oYkVgrFMAuc) (2016.01.12)
* [OPEL Demo Video: Nvidia TX1](https://www.youtube.com/watch?v=MFXGAeuxfxE) (2016.10.06)

## [License](https://github.com/sinban04/opel-alpha/wiki/License)
OPEL is open source software under the [Apache 2.0 license](http://www.apache.org/licenses/LICENSE-2.0). Complete license and copyright information can be found within the code.

Copyright 2015-2017 CISS, and contributors.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
