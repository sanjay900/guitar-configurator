# Adding RGB LEDs to controllers
You can add LEDs that will light up when a button or analogue input is pressed. For people playing Clone Hero, you can also make the frets light up based on in-game events such as Star Power.
## You will need
* A configured wii adaptor or directly wired controller. Note that PS2 controllers are not supported, as they use the same pins as the APA102s.
* Some APA102s

## The finished product
{% include youtube.html id=H73McH5abes %}

## Steps
1. Connect the VCC and GND pins on all LEDs that are being used to the Arduino.
2. Connect the SCK and SCL on the first LED to the SCK and MOSI pins on your Arduino. Then chain the SCK and SCL outputs to the inputs on the next LED, until all LEDs are connected.
   * For an Arduino Pro Micro (3.3v), Leonardo or Micro, the SCK pin is pin 15, the MOSI pin is pin 16 and the MISO pin is pin 14.
   * For an Arduino Uno, pin 11 is MOSI, pin 12 is MISO and pin 13 is SCK.
3. Open the Config Tool and find your device, then click continue
4. Click on Configure LEDs
5. Set the LED Type to APA102
6. Click Close
7. Now, when configuring, you will have the option to enable LEDs for all inputs. Enable LEDs for the inputs you want to use. 
8. When LEDs are enabled, you can also configure different colours for each input. You will be presented with a colour picker, and the LEDs will change in real time as you change colours.
9. Click on Configure LEDs
10. You should have a Grid of all of the LEDs you have enabled. From this screen, you can drag and drop different LEDs around, so that the tool knows the order you have chained your leds together in. See the following image:
    ![Arduino uno in home screen](../assets/images/drag-led.png)
2. Click on Close
3. Click on Write