PLATFORM = lpc17xx
JTAG = stlink
USER_TESTS +=

purge-flash:
	make purge
	make program
