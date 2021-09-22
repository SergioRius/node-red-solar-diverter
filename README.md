# node-red-solar-diverter [![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?business=JRX9JK6SSY25N&no_recurring=0&item_name=Open+source+donations&currency_code=EUR)
#### _A combination of node-red flow and a minimal set of hardware that allows to divert solar generated energy to non electronic controlled devices, like a water heater or small heat accumulators._

## Features

- Minimal and cheap hardware parts required.
  With just a chinese AC Dimmer and an ESP8266 you can work out the project.
- All the involved functions and requirements integrated in just one flow.
- Can manage excess energy from both feed-in or zero-feed systems.
- Uses a PID controller for a smooth operation.
- Publishes a small dashboard for easily testing and adjust.

## Hardware

You will need an AC Dimmer rated for the required duty. I've been using the one  on the [following link](https://es.aliexpress.com/item/1005001965951718.html?spm=a2g0s.9042311.0.0.3cb763c0xwzS4K) (16A) with success with a 1500W water heater. Always when buying from chinese sources, chose a higher rating one. You should add a small fan to the heat-sink.

Then connect an ESP8266 to the dimmer and provide a suitable power source. You can power the board using a PSU or a phone charger.

The code for the ESP is contained at the root of the github repo. For the moment does not have any captive portal or other extra functionalities other than the minimal needed. Just edit the main file for entering your wifi SSID and Key and burn to the ESP using platformio.

The sketch will setup the ESP+Dimmer to receive the PWM setpoint via mqtt at the topic 'PWMController/value' and establish a time wachdog to avoid diverting if messages are not received for a period of time.

## Flow

![The flow](https://github.com/SergioRius/node-red-Solar-diverter/blob/main/flows/flow.png?raw=true)

The flow has three parts: The upper-center one has the programming logic for the divert manager. You should edit the mqtt output node at the right and edit to match the topic configured in your ESP. On the center section a PID algorithm is implemented along with a manual override.

On the lower-left region is the inputs section. You only need to feed the `in_Watts` input with the power source to monitor. Being grid feed-in, or battery watts in case of self-consumption.
There are implemented two possible ways of automating the diverter. The upper one uses the battery state (bulk/absorption/float) to dinamically set the diverting setpoint, but you could replace it with a SoC percentage logic. If you set a very high setpoint, will behave the same as it was turn off.
The last one turns on or off the system based on SoC. Could also be the daylight detection reported by some mppt controllers. This is useful in a zero-feedin system, as it could also have a nightly charging cycle, to avoid diverting in that cases.

The right section has the default values node, `node-red-contrib-set-defaults` node is required here, and publishes an small dashboard useful for adjusting the PID coefficients, supervising and manually overriding the functions of the flow. Just like in the image below:

![Control panel](https://github.com/SergioRius/node-red-Solar-diverter/blob/main/flows/dashboard.png?raw=true)

You can setup several PID implementations with their own parameters and switch them here and compare the results, as well as turning all off.

## TODOs:
- Making it a node-red node. :grin:

## Contribution

If that bit of work become useful to you, remember that you can make a donation or drop me a :beer:
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?business=JRX9JK6SSY25N&no_recurring=0&item_name=Open+source+donations&currency_code=EUR)