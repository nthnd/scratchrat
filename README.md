# scratchrat

Ultra minimal infinite scratch-pad for Linux.

![sample screen shot](demo.png)

## Description

I've always wanted a really simple scratchpad program that has the following (anti-)features:

- Focused
  - no tools, including:
    - eraser
    - brushes
    - selection tool
    - text input
    - color picker
  - no distracting UI elements like tool bars, status bars, grids, and such
- Reasonably quick
- Infinite canvas
  - not limited to a predetermined canvas size
- Simple controls
  - Draw (Left click)
  - Undo (`<c-z>`)
  - Zoom (scroll)
  - Pan (Right click and drag)

## Usage

Just run the program and use your mouse to draw.

### Customization

Edit the source and rebuild.

## Building

### Nix
Clone the repo and run `nix build`, or just `nix run github:nthnd/scratchrat`

### Without NIX
1. Clone the repo.
2. Make sure you have Raylib version > 5.0
3. Run `make scratchrat`
4. Copy the `scratchrat` binary somewhere into your `$PATH`

