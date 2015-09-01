# relabsd
Duplicates an input device to convert its relative axis into absolute ones.
## Why would you do that?
The main purpose of this program is to bypass an issue of 3DConnexion's SpaceNavigator (but other devices might also suffer from this issue).
Indeed, this device reports having relatives axis despite sending absolute values.
Here are some people having issues because of this:
* http://www.3dconnexion.com/forum/viewtopic.php?t=2688
* http://www.osculator.net/forum/archive/index.php/t-708.html
* http://www.gimpusers.com/forums/gimp-developer/6231-adding-spacenavigator-support#message29509

Relative axis make Simple DirectMedia Layer (SDL) consider the device as 2 dimentional track ball, whereas absolute ones are understood to be a 6 axis joystick (which, at least to me, seems about right).


## How?
### Dependencies
* CMake
* libevdev

### Compiling
```
$ mkdir build
$ cd build
$ cmake ..
```
### Running
```
# ./relabsd /dev/input/spacenavigator ../conf/space_navigator.conf
```
