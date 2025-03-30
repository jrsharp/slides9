# slides9

slides9 is a presentation tool for 9front. -- "You're a presentation tool!"
It displays Markdown slides as established by Maas Lalani's ```slides``` tool.

## Features

- Markdown formatting with support for:
  - Headers (H1-H6)
  - Lists
  - Code blocks with syntax highlighting
  - Tables
  - Block quotes
  - Horizontal rules
  - Images
- Automatic text wrapping to fit window width
- High contrast color scheme
- Environment variable based font configuration

## Usage

```
slides9 filename.md
```

## Environment Variables

slides9 supports the following environment variables:

- `font`: The main font to use (default: system default font)
- `fontlarge`: Larger font for headings (H1 and H2)
- `fontsmall`: Smaller font for H6 and other small text elements

Example:
```
fontlarge=/lib/font/bit/pelm/unicode.24.font slides9 presentation.md
```

## Keyboard Controls

- Left/Right arrows or h/l: Navigate between slides
- g: Go to first slide
- G: Go to last slide
- f: Toggle fullscreen
- q: Quit
- ?: Show help

