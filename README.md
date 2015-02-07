Fishy Fun Times
========================

This code is to drain and refill and aquarium sump from a large resevoir. It's safe to loop the sump into the resevoir since there's a test to make sure we don't overflow it with the sump is draining.

Anyway, this project uses the Arduino IDE and an Arduino Mini, but any Arduino should do. A state machine dicates whether the sump should be filling or emptying. Depending on that state, the Arduino turns on one of two relays to either a filling pump or an emptying pump. Super simple.

