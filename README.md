# mixtable
Modular Midi Controller using Arduino


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
