# PIO display driver for SH1106

## Why is there a need for a SH1106 driver?
The SH1106 lacks the nifty horizontal/vertical addressing modes that
auto-increments the row when the end of the column is reached and vice
versa. This leaves only the less advanced page addressing mode where
each page has to be explicitly selected after finishing writing the
previous page. This requires a command, which in turn requires the
data or command pin (D/C) to be toggled. This is not covered by the
SPI controller, so it means that a maximum of one row can be sent in
each DMA transaction. Between each row the CPU has to toggle the D/C
pin, create a DMA transaction for the commands to switch row, and then
send the next row. This is terribly inefficient interrupting the CPU
all the time to just change a pin between 1 and 0. If only there were
some way to make the SPI controller do this?

## How does the driver work?
The pio display driver does exactly this. It allows setting the D/C
bit within the DMA transaction. This is done by adding a small header
construct to the data. The top 16 bits are the number of 32 bit words
to send. The lower most bit is the D/C pin of the display. The 15 bits
in between are unused by the current implementation.

```
31 .  .  .  .  .  .  .  23 .  .  .  .  .  .  .  15 .  .  .  .  .  .  .  7  .  .  .  .  .  .  0
<        number of 32 bit words to send        ><               NOT USED                  ><D/C>

```

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

Where `blc` will build load and start the process.
