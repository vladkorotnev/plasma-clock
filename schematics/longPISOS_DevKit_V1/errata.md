# LongPIS-OS DevKit V1 Errata

* Other side of CS_ENA jumper not wired to GND
* AUTORST, AUTOBOOT, CS_ENA is open by default, must be closed
* Q1, Q2 are using suboptimal packages
* Other side of AUTORST, AUTOBOOT not wired to the actual ESP chip
* C1 is too close to U1, fairly tight fit
* Trace between REG5V and C3-C4 is thin somehow
* HC125 too close to connector U2, hard to solder (maybe cause I bought the wrong package of HC125 lol)
* REG5V is a NRD Murata module that is probably overkill (though the GU321 does eat quite some juice on 5V!)
* Peripheral connections (LIGHT, IRRECV, MOTION, THERMA) are too close to the edge to install an angled XH connector
* Internal peripheral connections (LIGHT, IRRECV, MOTION, BUZZ) would have been better off along the longer edges of the board
* "?" testpad under U2 is hard to measure with the ribbon plugged in
* Ceramic/tantalum footprint has legs too close together
* Would be useful to fit the PROGRAM header somewhere on the left side to be accessible when assembled