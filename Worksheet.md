Worksheet:
=====================

I/O:
---------------------

	DI 2  = Sump Full
	DI 3  = Sump Low
	DI 4  = Sump Empty

	DI 5  = Barrel Full
	DI 6  = Barrel Low
	DI 7  = Barrel Empty

	DO 11 = IN1 = Fill  pump
	DO 10 = IN2 = Drain pump

	DO 13 = Error status

States:
--------------------

	Draining:   1
	Filling:    2
	Idle:       0

Errors:
--------------------

	1:	Barrel isn't at a safe fill level
	2:	Barrel overflowed while emptying the sump
	3:	Barrel underflowed while filling the sump
	4:	Impossible state, the phyiscal properties of reality aren't holding!
	5:	Something took too long. Assuming water is leaking from system

