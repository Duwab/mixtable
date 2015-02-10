# Mixtable
**Modular Midi Controller using Arduino**

**Objective:** Only one master module is producing midi to send (and receive) to (from) a computer. The master module manages several slave modules which measure different kinds of sensors, as creative as possible. The connection between master/slave modules is made with a simple stereo jack cable (one line for the ground, one line for the positive supply and one line for the communication in both ways)

### Structure

```
src\                // code dedicated to the modular midi controller
 |
 |-- master\                      // master code (listening for several modules, sending midi, ...)
 |     |
 |     |-- master.ino
 |
 |-- slave\                       // slave programm (measuring sensors, specific interactions, ...)
 |     |
 |     |-- slave.ino
 |
 |-- common\                      // common functions (transmission protocol, transmission tension mesure, ...)
 |     |
 |     |-- common.ino
 |
sample\             // simple examples of programs whose code can be reused
 |
 |-- midi_instrument\             // simple midi instrument
 |     |
 |     |-- midi_instrument.ino
 |
 |-- midi_controller\             // simple midi controller
 |     |
 |     |-- midi_controller.ino
 |
 |-- drums_captor\                // simple drums captor
 |     |
 |     |-- drums_captor.ino
 |
 |-- bluetooth_transmission\      // TODO: bluetooth transmission between arduino and android device
 |     |
 |     |-- bluetooth_transmission.ino
```
