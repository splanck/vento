# Vento Text Editor

Welcome to Vento, the lightweight text editor that's all about keeping things simple and easy to use. Whether you're coding, jotting down notes, or working on your next big writing project, Vento has got you covered.

## Features We've Got So Far

Here's what Vento can do right now:

- **Basic Editor**: The essentials are here. Open up Vento and start typing away.
- **Load Files**: Open and edit your existing files without any hassle.
- **Save As**: Save your work with a new filename whenever you need to.
- **Status Bar**: Keep track of where you are in your document with our handy status bar.
- **Create New File**: Start fresh with a new file in just a few keystrokes.
- **Scroll Bar**: See how far you've scrolled through your document.
- **Help Screen**: Press `CTRL-H` for a quick guide to Vento's features.
- **Save Feature**: Save your changes without having to re-enter the filename every time.
- **Basic Syntax Highlighting**: Enjoy very basic poorly implemented syntax highlighting.
- **Undo and Redo**: Because mistakes happen, and fixing them should be easy.

## What's Coming Up (Maybe)

We've got some cool ideas on our to-do list. No promises, but here's what we're thinking:

- **Syntax Highlighting**: Proper syntax highlighting support.
- **Support Multiple Files**: Work on several files at once without breaking a sweat.
- **Spell Checker Support**: Catch those pesky typos automatically.
- **Copy and Paste**: Basic clipboard functionality to move text around.
- **Git Integration**: Seamlessly integrate with Git for version control.
- **Menu System**: Navigate Vento's features through an intuitive menu.
- **Customizable Configuration**: Tailor Vento to your liking with custom settings.
- **Command to Delete Current Line**: Quickly delete lines you don't need.

## Prerequisites

Before you can compile Vento, make sure you have the following installed:

- **GCC**: The GNU Compiler Collection for compiling the source code.
- **Binutils**: A collection of binary tools.
- **Ncurses Dev Libraries**: Development libraries for Ncurses (necessary for text-based user interfaces).

On Debian-based systems, you can install these with:

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
```

## Getting Started

Ready to give Vento a try? Here's how to get started:

```bash
git clone https://github.com/yourusername/vento.git
cd vento
make
make install
```

Thanks for checking out Vento! Happy editing!
