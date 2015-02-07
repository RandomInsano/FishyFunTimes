Fishy Fun Times
========================

This code is to control a water cycling system in an aquarium setup of a family member. The whole point is to drain out one container, and refill it from a second, much larger container. There's a test in the code while draining the first container that the second container won't overflow, so if you wanted to you could use this in a safe loop.

In any event, this project uses the Arduino IDE and a standard ATMega328P programmed with the Arduino bootloader. The most important part of the project is the state machine which dicates whether the sump tank should be filling or emptying. Depending on that state, the Arduino turns on one of two relays to either a filling pump or an emptying pump.

