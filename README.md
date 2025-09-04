# CHIP-8 Emulator in C

This is a fully functional, highly modular CHIP-8 emulator written from scratch in C using the SDL2 library for graphics and input. This project was undertaken as a practical exercise to deepen understanding of C programming, low-level computer architecture, and the fundamentals of emulation development.

The emulator correctly implements all 35 standard opcodes of the CHIP-8 virtual machine and is capable of playing classic games from the era.

---

## Features

- **Full Instruction Set:** Complete and accurate implementation of all 35 CHIP-8 opcodes.  
- **Graphics and Input:** Utilizes the cross-platform SDL2 library to handle the 64x32 monochrome display and the 16-key hexadecimal keypad.  
- **Configurable Emulation:** Control the emulator's behavior via command-line arguments, including:
  - Custom CPU clock speed (`--clock-rate`).
  - Adjustable display scaling (`--scale`).  
- **Debugging Tools:**
  - **Trace Logger:** A console output that logs the machine state (PC, opcode, V-registers) for each CPU cycle.
  - **Step-Through Mode:** A special mode (`--step`) to pause execution and advance one instruction at a time, perfect for detailed analysis.
- **Legacy Support:** Includes support for legacy hardware quirks (such as the I register behavior in Fx55/Fx65) to ensure compatibility with older ROMs.

---

## Architectural Highlights

This project was built with a focus on clean, modern C architecture and professional design patterns.

- **Modular Design:** The code is cleanly separated into a core virtual machine (`chip8.c`), a platform host (`main.c`), and a configuration parser (`config.c`).  
- **Function Pointer Dispatch:** Opcodes are handled via a jump table (an array of function pointers) for efficient and highly readable instruction dispatch, avoiding a monolithic switch statement.  
- **PC Control Signaling:** A robust system where opcode handlers signal to the main loop whether they have taken control of the Program Counter, allowing for clean implementation of jumps, calls, skips, and returns without code duplication.  

---

## Building and Running

### 1. Dependencies

You will need a C compiler (`gcc` or `clang`), `make`, and the SDL2 development library.  

On Debian-based systems (like Ubuntu or WSL), you can install the dependencies with:

```bash
sudo apt update
sudo apt install build-essential libsdl2-dev
````

---

### 2. Building

The project uses a standard Makefile. To compile, simply run `make` from the root directory:

```bash
make
```

This will create an executable at `build/chip8_emulator`.
To clean up build files, run:

```bash
make clean
```

---

### 3. Running

To run the emulator, you must provide the path to a CHIP-8 ROM file.

**Basic Usage:**

```bash
./build/chip8_emulator path/to/your/rom.ch8
```

For example, if you have a `roms/` directory:

```bash
./build/chip8_emulator roms/PONG
```

---

### Command-line Options

The emulator's behavior can be modified with the following flags:

| Short | Long         | Argument   | Description                                      |
| ----- | ------------ | ---------- | ------------------------------------------------ |
| -h    | --help       |            | Show the help message and exit.                  |
| -s    | --step       |            | Enable step-through debugging mode.              |
| -c    | --cycles     | `<count>`  | Run for a specific number of cycles, then exit.  |
| -r    | --clock-rate | `<hz>`     | Set the CPU clock speed in Hertz (default: 500). |
| -S    | --scale      | `<factor>` | Set the display scale factor (default: 10).      |

**Example with options:**

```bash
./build/chip8_emulator --clock-rate 700 --scale 15 roms/INVADERS
```

---

## Controls

The 16-key CHIP-8 keypad is mapped to the left side of a standard QWERTY keyboard:

| CHIP-8  | Keyboard |
| ------- | -------- |
| 1 2 3 C | 1 2 3 4  |
| 4 5 6 D | Q W E R  |
| 7 8 9 E | A S D F  |
| A 0 B F | Z X C V  |

---

## Planned Features

This project is a solid foundation, and there are several planned enhancements to make it a more complete and feature-rich emulator:

* **Testing:** Perform tests using test roms like [Timendus/chip8-test-suite](https://github.com/Timendus/chip8-test-suite) and publish the results. In case of failing tests, make it a priority to get them working.
* **Sound Support:** Implement beep sounds when the sound timer (ST) is active using SDL\_mixer or a similar library.
* **Advanced Debugging Tools:**

  * Create a live memory viewer in a separate SDL window to inspect the full 4KB memory space in real-time.
  * Develop a full disassembler to display human-readable instructions in the trace log.
* **Configurable Quirks:** Add a command-line flag to toggle legacy hardware behaviors (like the I register modification) for maximum ROM compatibility.
* **Save/Load State:** Implement functionality to save the current state of the virtual machine to a file and load it back later.
