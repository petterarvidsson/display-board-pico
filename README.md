# PIO display driver for SH1106

## Why is there a need for a SH1106 driver?
The SH1106 lacks the nifty horizontal/vertical addressing modes of the
SSD1306 that auto-increments the row when the end of the column is
reached and vice versa. This leaves only the less advanced page
addressing mode where each page has to be explicitly selected after
finishing writing the previous page. This requires a command, which in
turn requires the data or command pin (D/C) to be toggled. This is not
covered by the SPI controller, so it means that a maximum of one row
can be sent in each DMA transaction. Between each row the CPU has to
toggle the D/C pin, create a DMA transaction for the commands to
switch row, and then toggle the D/C pin again, and then send the next
row. This is terribly inefficient interrupting the CPU all the time to
just change a pin between 1 and 0. The pio display driver solves
exactly this problem.

## How does the driver work?
It allows setting the D/C bit within the DMA transaction. This is done
by adding a 32 bit header to the data sent to the PIO via DMA. The 16
MSBs are the number of 32 bit words to send. The LSB (the lowermost
bit) is the D/C pin of the display. The 15 bits in between are unused,
but must all be set to zero in the current implementation.

```
31 .  .  .  .  .  .  .  23 .  .  .  .  .  .  .  15 .  .  .  .  .  .  .  7  .  .  .  .  .  .  0
<        number of 32 bit words to send        ><         NOT USED, MUST BE ZERO          ><D/C>

```

This way a full display update can be done within a single DMA
transaction, leaving the CPU to other things in the meantime. An
example of this (including the initialization of the display) can be
seen [here](src/pio-display.c#L26).

## Building

It can be built like any pico project using cmake

### Generate makefile
```
mkdir build
cd build
cmake ..
```

### Debug via gdb

There is a gdb command file included to make debugging using openocd
easier. (It automatically connects openocd, no need to start it
separately)

```
gdb-multiarch -x gdbcommands
...
blc
```

Where `blc` will build, load and start the process.
