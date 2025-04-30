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

## Troubleshooting

### Window Doesn't Open

If the application builds successfully but the window doesn't appear:

1. Make sure all required DLLs are available
   - For MinGW, ensure `libgcc_s_dw2-1.dll` and `libstdc++-6.dll` are in PATH or copied to build directory
   - For Visual Studio, ensure Visual C++ Runtime is installed

2. Check for missing dependencies
   - GDI+ is required for image handling
   - Make sure graphics drivers are up to date

3. Try running in debug mode
   - Edit build_and_run.bat and add `--debug` to the command line
   - This will show additional console output for troubleshooting

4. Look for Windows Event Viewer errors
   - Check Application logs for any crashes or errors

### Build Errors

Common build errors and solutions:

1. "Cannot find -lXXX" error:
   - Missing library. Install the required development package.

2. "XXX.h: No such file or directory":
   - Missing header file. Check include paths.

3. Linker errors:
   - Make sure all source files are included in the build command.
   - Check library paths and dependencies.

## License

This project is provided as-is with no specific license.
