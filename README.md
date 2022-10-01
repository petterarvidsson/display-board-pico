# Building

## Generate makefile
```
mkdir build
cd build
cmake ..
```

## Debug via gdb

```
gdb-multiarch -x gdbscript

blc
```

Where `blc` will build load and start the process.
