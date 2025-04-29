# PixelForge

A simple and elegant image editor with a clean UI.

## Features

- Multiple predefined canvas resolutions (HD, Full HD, QHD, 4K)
- Custom 1280x750 resolution preset
- Open and edit images
- Clean, modern interface

## Building the Project

### Prerequisites

You need either:
- MinGW with g++ compiler, or
- Visual Studio with C++ development tools

### Building

Simply run the `build_and_run.bat` script to build and run the application:

```
build_and_run.bat
```

This will automatically detect your compiler, build the project, and run it.

### Project Structure

```
PixelForge/
├── build/                # Build output directory
├── src/                  # Source code
│   ├── core/             # Core application logic
│   ├── ui/               # User interface components
│   ├── utils/            # Utility functions
│   └── resources/        # Resource files
├── build_and_run.bat     # Build script
└── README.md             # This file
```

## Development

The application is structured in a modular way:
- `src/main.cpp` - Entry point 
- `src/core/application.*` - Main application class
- `src/ui/main_window.*` - Main window UI implementation

## License

This project is provided as-is with no specific license.
