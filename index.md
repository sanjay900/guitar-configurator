---
layout: default
---

The Ardwiino Guitar Configurator tool allows you to program Arduinos for your own custom controllers.

![alt text](assets/images/main-screen-graphical.png)

# Features
* Controllers can emulate a variety of different controllers, including XInput (not Compatible with xboxs), PS3, Nintendo Wii Rockband Controllers, PS4 Rockband Controllers and even a Nintendo Switch Pro Controller
* Controllers can either take input from to Wii controllers, take input from to PS2 controllers or the inputs can be wired directly to the Arduino
* Controllers poll at <1ms, but due to the limitations of USB 2.0, this will result in a poll rate of 1ms
* Controllers support having LEDs react to inputs, including using APA102s (Dotstar) or just wiring regular LEDs inline with the buttons.
* An Arduino Pro Micro, Arduino Micro, Arduino Leonardo, Arduino Uno or Arduino Mega can be used.
* Tool is cross platform, with LED support working on Linux, macOS and Windows.
* A variety of tilt switches can be used in order to support Star Power tilt for Guitar Hero Guitars, including mercury switches, analogue accelerometers and the MPU-6050

```note
Avoid using USB 2.0 hubs when using these controllers, they poll so quickly that multiple guitars on a single hub will be slowed down, and some hubs will even cause issues with the controller being detected. However, USB 3.0 hubs are okay, and I have previously used multiple guitars on a single USB 3.0 hub with no issues.
```

# How to build an adaptor or controller
Pick a guide from the left to learn how to build a controller or adaptor.

# Need Help? Something not working?
Feel free to message me on Discord, my username is sanjay900#5549.

# Download link
Click [Here](https://github.com/sanjay900/guitar-configurator/releases/latest) to download. You can visit [this link](https://github.com/sanjay900/guitar-configurator/releases) in order to see older releases and testing releases.

# Gallery
{% include youtube.html id=H73McH5abes %}
<img src="assets/images/adaptor.jpg" height="315">
<img src="assets/images/adaptor-ps2.jpg" height="315">
<img src="assets/images/direct.jpg" height="315">
<img src="assets/images/inline-led.jpg" height="315">

<!-- 
We can do this, but currently it isnt useful as we dont have proper descriptions
# Changelog
{% for release in site.github.releases %}
  * [{{ release.name }} - {{ release.body }}]({{ release.html_url }})
{% endfor %} -->

# Donate Links
I have put several years of development time into this project, if you like it feel free to support this project by clicking either of the links below.
{% include sponser.html %}
[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=TFL9DSUEF7VN8&source=url)