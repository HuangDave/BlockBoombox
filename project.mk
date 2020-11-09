PLATFORM = lpc17xx
JTAG = stlink
OPT = g

USER_TESTS +=

purge-flash:
	make purge
	make program

app:
	make purge
	make application

purge-debug:
	make purge
	make program
	make debug