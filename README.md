# EEET2490 Group Project To-Do List

## 1. Welcome Message and Command Line Interpreter (CLI) (~30%)
- [x] Create ASCII welcome message using online tool  
- [x] Display OS name as CLI prompt (e.g., `MyOS>`)  
- [x] Implement command input buffer with Enter key detection  
- [x] Support auto-completion with TAB key  
- [x] Implement command history (UP = `_`, DOWN = `+`)  
- [x] Allow backspace/deletion without removing OS name  

### Commands to implement
- [x] `help` – list and explain commands  
- [x] `clear` – clear/scroll the screen  
- [x] `showinfo` – show board revision + MAC address  
- [] `baudrate` – allow changing UART baud rate  
- [] `handshake` – enable/disable UART CTS/RTS  

## 2. Image, Video, and Text Display (~20%)
- [x] Display all team member names with background image  
- [x] Use custom fonts and different colors for text  
- [x] Record and display a short video on screen  
- [x] Convert video into frames and display sequentially  
- [x] Create bitmap font data for character rendering  

## 3. Game Application for Bare Metal OS (~50%)
- [] Design and implement a small game with graphics and colors  
- [] Implement at least one fixed game stage (preferably more)  
- [] Enable CLI-terminal control input (ACK/NAK feedback)  
- [] Implement data logging (e.g., command count, state info)  
- [] Ensure user-friendly gameplay with effective UI  
- [] Test the game on both QEMU and real hardware  

## 4. Report and Presentation
- [] Write a complete report with background, results, screenshots  
- [] Discuss project success and limitations  
- [] Record demo video (max 20 mins, all members present)
