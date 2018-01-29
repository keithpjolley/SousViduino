# SousViduino
Yet Another Sous Vide project for the Arduino

This is a project that has been in the works for several years. See below for the long sad saga.

## Setup
My setup is an Arduino Uno (though it compiles/runs on a Mega without changes too).

On top that is a proto-screw shield similar to this:
http://www.kumantech.com/kuman-screw-shield-expansion-board-for-arduinouno-r3-ky02-2pcs_p0052.html

For the LCD display I'm using a plain LCD/Keypad shield (hardware rev 1.0) 
https://create.arduino.cc/projecthub/niftyjoeman/osepp-lcd-and-keypad-shield-d5b46e

The thermometer is a one-wire DS18B20. It says it's waterproof but I guess you run the risk of
having nasty plastics leach into your food unless it's wrapped it in food-grade plastic tubing.
https://www.adafruit.com/product/642
https://www.adafruit.com/product/1020

The relay I'm using is an "8 Relay Module". A smaller relay (like only one) would work fine but this is what I had on hand.
It's important that the high-voltage (output) side of the relay is electrically isolated from the low-voltage (input) side.
I'm not sure who made mine but it is similar to this:
https://www.dx.com/p/8-channel-5v-relay-module-board-for-arduino-red-156424

I also have a small breadboard but with the screw-shield it's probably not needed. I had my thermometer,
I mean "Digital Temperature Sensor," already wired into the breadboard so this is the path of least resistance.
I haven't gotten around to soldering in the required resistor yet. (If a resistor is required why doesn't it
come from the factory with one installed?)

## Wiring
I have the thermometer on pin 11 and the relay on pin 12. Not much to it. The relay is inserted as a bump-in-the-wire,
the wire being an old extension cord I sliced open. One end of the cord goes into the wall, the crockpot is plugged into
the other end.

## Testing
For initial testing I used a kitchen thermometer to see if they agreed on the ambient temperature - they did. 
To check if the PID circuit was working I plugged a desklamp into the extension cord and put the thermometer near
bulb and set the `targetTemp` to 80°F. Here's some sample Serial output:
<pre>
targetTemp: 80.00°F, actualTemp: 79.93°F, output: 0.23, kp: 2.00, ki: 0.50, kd: 2.00, relay: ON
targetTemp: 80.00°F, actualTemp: 80.04°F, output: 0.00, kp: 2.00, ki: 0.50, kd: 2.00, relay: OFF
targetTemp: 80.00°F, actualTemp: 79.93°F, output: 2.48, kp: 2.00, ki: 0.50, kd: 2.00, relay: ON
targetTemp: 80.00°F, actualTemp: 80.04°F, output: 0.00, kp: 2.00, ki: 0.50, kd: 2.00, relay: OFF
targetTemp: 80.00°F, actualTemp: 80.04°F, output: 0.01, kp: 2.00, ki: 0.50, kd: 2.00, relay: ON
targetTemp: 80.00°F, actualTemp: 80.04°F, output: 0.00, kp: 2.00, ki: 0.50, kd: 2.00, relay: ON
targetTemp: 80.00°F, actualTemp: 80.04°F, output: 0.00, kp: 2.00, ki: 0.50, kd: 2.00, relay: OFF
</pre>
I'm not 100% convinced that the "Press 'SELECT'" to retune does anything but switching the temperature
with the UP|DOWN buttons works fine. I'm not concerned if the retune works or not right now so I'm
not going to worry about it for now. It's not a problem that needs to be solved.

## Long Sad Saga
I love everything about https://www.adafruit.com except for one thing. A lot of their stuff needs soldering.
I'm not good at soldering fine things. I can solder 12 guage wire all day long, or garden irrigation no
problem. But soldering anything on to a PCB is beyond my skills. I burn the chip, the board, the circuit, my
fingers, everything but the solder. I had originally ordered all the parts needed for this project several
years ago from adafruit but it took a month or more to get around to working on it and that's when I discovered
the fancy RGB LCD needed to be soldered. My first pass I ended up soldering the entire thing into one large
pile of carbonized PCB and molton solder. Needless to say the magic smoke got out so I bought another and got
an appropriate soldering kit. 

The second time worked a little better. The magic smoke stayed in and I could get the RGB background to change
colors but I couldn't read anything on the display. Tech support said "blah blah blah bad connection on pin blah"
which I already knew and had tested continuity on but it didn't work. 

So I ordered a third display but I must have had Pi on the brain because it was an RPi model. Cool but not what
I needed for this project.

I ordered a pre-soldered LCD display (link above) and then got around to running the code from the adafruit sous
vide project https://learn.adafruit.com/sous-vide-powered-by-arduino-the-sous-viduino and of course it made
use of the RGB colors. Easy enough to rip out the color coding but then I realized that the tuning nonsense
wasn't something I was interested in doing and I definitely wasn't going to go through more than once so the
project sat on the back of my desk for another couple of months.

At some point I remembered I had a WiFi shield so I figured it'd be easy to create a web interface and do
all the control from my desktop, however, I couldn't get the WiFi shield to come to life. Even an old project
that worked just fine on the exact same hardware wouldn't work. Bummer. Back of the desk again.

In the meantime I got an ESP8266 4 Relay module. I wasted a few days trying to get the Linksprite software
to work then wrote my own interface. I'll try to remember to update this with a link to that code as soon as
I post it.

So, finally, I realized I really wanted to get this project done so I started from scratch. With a blank
Arduino project I made it so I could read the temperature sensor. Next I added a display to show what
the temperature was. Turning the relay on/off based on the temperature was next - on if the temp was below
a preset and off otherwise. Then I added the PID software and remembered I had no interest in tuning it
or decoding the terrible/non-existant documentation so I inserted the AutoTune code. I thought I was done
but I added in being able to change the temp on the fly and removed any unused code/vars.

Next step will be to see if I can connect the one-wire directly to the ESP8266 relay module. That'd be cool.
















https://www.adafruit.com/product/714

