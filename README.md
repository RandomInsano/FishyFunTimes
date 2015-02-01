Fishy Fun Times
========================

This code is to control a water cycling system for a family member. The whole point is to cycle the water from one container to a second, much larger container. I'm assuming to let some of the aquarium water stagnate and settle. I'll need to ask. :P

In any event, this project uses the Arduino IDE and a standard ATMega328P programmed with the Arduino bootloader. The most important part of the project is the state machine which dicate whether the sump tank should be filling or emptying. Depending on that, it turns on one of two relays to control either a filling pump or an emptying pump.

This is the first project I've done via the Arduino IDE. It's likely going to be the last. I still prefer having either a larger IDE or a series of Makefiles. The Arduino abstraction is pretty nice however.
