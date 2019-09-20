10seg fullcolor (OSX10201-LRPB2) I2C driver using attiny2313
====

## Description
A program 10seg fullcolor (OSX10201-LRPB2) I2C driver using attiny2313.  
(Atmel Studio 7 project files)  

By using the I2C command, you can easily turn on and off any LED with any color.  
Details of the I2C command can be found in the following PDF.  

[10segcolorled_i2c_attiny2313.pdf](./docs/10segcolorled_i2c_attiny2313.pdf)  

The operation of this program has been confirmed using the i2cget / i2cset command of Raspberry Pi 3.  
In some cases, commands may not be received.

## Demo Circut

![sample](./img/sample.jpg)

```
    ATtiny2313 fuse factory default LOW:0x64 HIGH:0xDF EXT:0xFF LOCK:0xFF
    I2C Address(Default): 0x55

    port mapping [ATtiny2313]

    bit      7    6    5    4    3    2    1    0
    PORTA    -    -    -    -    -    -    -    -
    PORTB    -    r    -    b    g    J    I    H
    PORTD    -    G    F    E    D    C    B    A

    10seg fullcolor [OSX10201-LRPB2]

    ABCDEFJHIJ (VCC)
    rbg----gbr (GND) -> The side where the string is printed

    r - 150R - GND
    b - 100R - GND
    g - 100R - GND
```

## Reference

* [usiTwiSlave.zip by @jsdiy](http://jsdiy.webcrow.jp/usitwis_clcd/)
* [Application Note AVR312: Using the USI Module as a I2C Slave](http://ww1.microchip.com/downloads/en/Appnotes/Atmel-2560-Using-the-USI-Module-as-a-I2C-Slave_ApplicationNote_AVR312.pdf)
* [AVR312 source files](https://www.microchip.com/wwwAppNotes/AppNotes.aspx?appnote=en591197)
* [OSX10201-LRPB2 DataSheet](./docs/OSX10201-LRPB2.pdf)

## Licence

* [rimksky][] CreativeCommon BY 4.0
* usiTwiSlave.c / usiTwiSlave.h  
  created by [atmel.com]  
  modifyed by [@jsdiy]  
  modified by [rimksky]  

## Author

* [rimksky][]

[rimksky]: https://github.com/rimksky "rimksky"
[jsdiy]: http://jsdiy.webcrow.jp/ "@jsdiy"
[atmel.com]: https://www.microchip.com/ "atmel.com"
