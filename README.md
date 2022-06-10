![](screenshots/test_opcode.png "test-opcode-ch8")

Download source code
```shell
$ git clone git@github.com:derekdai/chip8.git
```

Install dependencies (`libsdl2-dev` and `systemtap-sdt-dev` are optional)
```shell
$ apt-get install -y \
  build-essential \
  meson \
  libsdl2-dev \
  systemtap-sdt-dev
```

Build with verbose log messages and enable trace points
```
$ cd chip8
$ meson build . -Dlog-level=127 -Denable-dtrace=true
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

Run arbitrary `.ch8` images, press `ESC` to stop
```shell
$ build/src/chip8 images/IBM\ Logo.ch8
```

To measure time of every single steps with `bpftrace`, run this command in a terminal
```shell
$ sudo bpftrace -e '
  usdt:build/src/libchip8.so:exec_begin {@begin = nsecs;}
  usdt:build/src/libchip8.so:exec_end {printf("%dus\n", (nsecs - @begin) / 1000);}'
```

Then run `chip8` command in another terminal
```shell
$ build/src/chip8 images/IBM\ Logo.ch8 30
```

Key mapping (not configurable yet)
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
