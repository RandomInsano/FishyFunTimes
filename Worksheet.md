Worksheet:
=====================

Consider this kind of part user manual, part schematic thinggy.

Usage:
---------------------

The start button on the system will start the sump's draining pump. When we detect the sump is empty, it we switch over to the filling pump until we detect the sump is full and turn off. At any point during this, you can cycle to the next step using the start button.

There are a number of other sensors (tank full, tank empty, etc) and if we detect a problem in the system, both pumps will shut off and the status LED will blink a count of whatever problem we hit. See below for a list of possible errors.

I/O Pin Setup:
---------------------

	DI 1  = Start, Step, Clear error

	DI 4  = Sump Full
	DI 5  = Sump Low
	DI 6  = Sump Empty

	DI 7  = Barrel Full
	DI 8  = Barrel Low
	DI 9  = Barrel Empty

	DO 11 = IN1 = Fill  pump
	DO 10 = IN2 = Drain pump

	DO 13 = Error status

States:
--------------------

	Idle:       0
	Draining:   1
	Filling:    2
	Error:		3

Errors:
--------------------

	1:	Barrel isn't at a safe fill level
	2:	Barrel overflowed while emptying the sump
	3:	Barrel underflowed while filling the sump
	4:	Impossible state, the phyiscal properties of reality aren't holding!
	5:	Something took too long. Assuming water is leaking from system

