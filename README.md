# EEET2490 Group Project – Bare Metal Operating System

## Project Overview
This project is developed as part of EEET2490 – Embedded System: OS and Interfacing (Semester 2025-1). It demonstrates the creation of a bare-metal operating system for the Raspberry Pi platform, featuring a custom command-line interface (CLI), framebuffer-based multimedia display, and a mini game application.

**Main Features:**
- ASCII welcome banner and interactive CLI (FixingGoodOS>)
- UART-based command parsing with auto-completion and history
- Display of background images, custom fonts, and text
- Frame-by-frame video playback using converted BMP images
- A basketball-themed game with player input, multiple items, and win/loss conditions

## Project Components
### 1. Welcome Message & Command Line Interface (CLI)
- Displays an ASCII boot message and custom prompt
- Supports built-in commands:
  - `help` – list commands or show usage
  - `clear` – clear screen via ANSI
  - `showinfo` – display board revision and MAC address via mailbox
  - `baudRate` – change UART baud rate (e.g., 9600, 115200)
  - `handShake` – toggle UART hardware handshake
  - `teamDisplay` – show team names using framebuffer
  - `videoDisplay <ak/cow>` – play selected video
  - `game` – launch game loop
- Features:
  - Tab-based command auto-completion
  - Command history (`_` and `+`)
  - Backspace support without erasing CLI prefix

### 2. Image, Video, and Text Display
- Team member names rendered using an 8x15 custom font (Tamzen)
- Background images rendered via ARGB32 framebuffer
- Video playback: Cow Dance and Gun Reload (stored as image arrays)
- Video runs at ~10 FPS, looped until Enter is pressed

### 3. Basketball Game Application
- Player-controlled hoop using UART input ('a' and 'd')
- Items fall from above (Normal, Special, Bomb, Enlarge, Multiply)
- Game stages: Street → School → NBA
- Win condition: pass all 3 stages
- Lose condition: score < -100 or time limit exceeded
- UART logs ACK/NAK messages, score updates

## How to Build & Run

### Requirements
- `qemu-system-aarch64` (QEMU emulator)
- `aarch64-none-elf-gcc`, `ld`, `objcopy`

### Compilation Steps
1. Open terminal and navigate to `./src`
2. Run:
   ```bash
   make
