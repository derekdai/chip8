Download and build from source
```shell
$ git clone git@github.com:derekdai/chip8.git
$ cd chip8
$ meson build .
$ meson compile -C build
```

Run nop example
```shell
$ build/examples/nop
```

Show IBM logo
```shell
$ build/examples/ibm-logo
```

Run `.ch8` image, press `ESC` to stop
```shell
$ build/src/chip8 images/IBM\ Logo.ch8
```

Key mapping
```
      Chip8            PC Keyboard
+---------------+   +---------------+
| 1 | 2 | 3 | C |   | 1 | 2 | 3 | 4 |
+---+---+---+---+   +---+---+---+---+
| 4 | 5 | 6 | D | > | Q | W | E | R |
+---+---+---+---+   +---+---+---+---+
| 7 | 8 | 9 | E |   | A | S | D | F |
+---+---+---+---+   +---+---+---+---+
| A | 0 | B | F |   | Z | X | C | V |
+---------------+   +---------------+
```
