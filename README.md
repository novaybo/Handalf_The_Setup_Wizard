
# Handalf The Setup Wizard

🧙 **Handalf The Setup Wizard** is a terminal-based interactive setup tool. It guides users through configuration and setup tasks in a conversational, wizardly manner—making system setup feel like a magical adventure!

## Features

- **UI:** Status and info messages appear in a flow, stacked like a chat log.
- **Terminal-based:** Runs in any terminal using ncurses for a retro, immersive experience.
- **Step-by-step guidance:** Handalf walks you through each setup step, providing feedback and instructions.
- **Colorful feedback:** Uses color to distinguish between info, success, and error messages.
- **Easy to extend:** Modular code structure for adding new setup steps or customizing the wizard's personality.


## Getting Started

### Prerequisites

- GCC or any C compiler
- `make`
- `ncurses` library (install via your package manager, e.g., `sudo apt install libncurses5-dev`)

### Build

```sh
make
```

### Run

```sh
./handalf_wizard
```

## Usage

- Use keyboard input as instructed to proceed through each setup step.
- Watch as Handalf logs each action and messages.

## Project Structure

```
Handalf_The_Setup_Wizard/
├── wizard.c         # Main source code
├── wizard.h         # Header file
├── Makefile
├── README.md
└── docs/
    └── screenshot.png   # If I put one
```

## Customization

- **Steps:** Add or modify setup steps by editing the main logic in `wizard.c`.

## Contributing

Pull requests and suggestions are welcome!  
Feel free to open issues for bugs or feature requests.

## License

MIT License

---

*May your setup be ever magical!*
