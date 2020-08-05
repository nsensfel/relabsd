# relabsd
 Turns your input devices into joysticks by converting relative axes into absolute ones.
 Additionally, some options let you create a modified copy of your relative device.
 
## Use cases
* Use a relative input device as a joystick.
* Bypass an issue causing your device to report its axis are relative when they are absolutes (see [the wiki page on 3DConnexion devices](https://github.com/nsensfel/relabsd/wiki/3Dconnexion-devices) for example), this can be required by some librairies (such as Simple DirectMedial Layer - SDL) for your device to be recognized.
* Change what controls which axis (still has to be on the same input device).
* Invert an axis.

## Dependencies
* CMake (>= 3.0)
* libevdev

## How to use
See [the related wiki page on how to set up relabsd](https://github.com/nsensfel/relabsd/wiki/Setting-up-relabsd), and the one on [how to create a virtual device](https://github.com/nsensfel/relabsd/wiki/Creating-a-virtual-device).

### Results
(Using [Grumpel's sdl-jstest](https://github.com/Grumbel/sdl-jstest))

Note that the real device (a 3Dconnexion SpaceNavigator, that's a 6DOF device) was made to report itself as being a joystick so it could be see by SDL. This is not required (only the virtual device has to).

**SDL 1:**
```
$ ./sdl-jstest --list
Found 1 joystick(s)

Joystick Name:     'relabsd: 3Dconnexion SpaceNavigator'
Joystick Number:    0
Number of Axes:     6
Number of Buttons:  2
Number of Hats:     0
Number of Balls:    0
```
**SDL 2:**
```
$ ./sdl2-jstest --list
Found 2 joystick(s)

Joystick Name:     '3Dconnexion SpaceNavigator'
Joystick GUID:     030000006d04000026c6000011010000
Joystick Number:    0
Number of Axes:     0
Number of Buttons:  2
Number of Hats:     0
Number of Balls:    1
GameController:
  not a gamepad

Joystick Name:     'relabsd: 3Dconnexion SpaceNavigator'
Joystick GUID:     030000006d04000026c6000011010000
Joystick Number:    1
Number of Axes:     6
Number of Buttons:  2
Number of Hats:     0
Number of Balls:    0
GameController:
  not a gamepad
```

# Troubleshooting
## I can't find my (real) input device.
Your input device should be listed in /dev/input. You can use commands such as `relabsd -? /dev/input/event8` to obtain information on the devices found there.

If you are using a 3DConnexion device and not finding it in /dev/input, read [this wiki page](https://github.com/nsensfel/relabsd/wiki/3DConnexion-devices).
