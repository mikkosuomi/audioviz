# Audio Visualizer Engine

A high-performance audio visualization engine written in C++ with Lua scripting support.

## Features

- Fast and efficient audio processing with FFT analysis
- High-performance OpenGL rendering
- Scriptable visualizations using Lua
- Cross-platform (Windows, macOS, Linux)
- Borderless window mode with dragging and fullscreen support
- Real-time audio reactivity

## Requirements

- CMake 3.12+
- C++17 compatible compiler
- OpenGL 3.3+
- SDL2
- Lua 5.3+ (optional but recommended)

## Building

```bash
# Clone the repository
git clone https://github.com/your-username/audio-visualizer.git
cd audio-visualizer

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . --config Release

# Run the visualizer
./bin/AudioVisualizer
```

## Visualization Scripts

The engine uses Lua scripts to define visualizations. Scripts are loaded at runtime and control how the audio is visualized.

### Script Lifecycle

Each script should implement the following functions:

- `onInit()` - Called when the script is first loaded
- `onUpdate(deltaTime, audio)` - Called every frame to update visualization state
- `onRender(renderer)` - Called every frame to render the visualization
- `onShutdown()` - Called when the script is unloaded

### Example Script

```lua
-- Simple particles visualization
function onInit()
    -- Initialize visualization
    return true
end

function onUpdate(deltaTime, audio)
    -- Update visualization state based on audio data
    return true
end

function onRender(renderer)
    -- Draw the visualization
    return true
end

function onShutdown()
    -- Clean up
    return true
end
```

### Available Audio Data

The `audio` parameter passed to `onUpdate` contains:

- `audio.energy` - Overall audio energy (0.0-1.0)
- `audio.bass` - Bass energy (0.0-1.0)
- `audio.mid` - Mid-range energy (0.0-1.0)
- `audio.treble` - Treble energy (0.0-1.0)
- `audio.spectrum` - Full frequency spectrum (array)
- `audio.waveform` - Time-domain waveform (array)

### Drawing Functions

The `renderer` parameter passed to `onRender` provides:

- `drawLine(x1, y1, x2, y2, color, thickness)`
- `drawCircle(x, y, radius, color, thickness)`
- `drawFilledCircle(x, y, radius, color)`
- `drawRect(x, y, width, height, color, thickness)`
- `drawFilledRect(x, y, width, height, color)`
- `drawParticle(x, y, size, color, shapeType)`
- `drawWaveform(samples, x, y, width, height, color)`
- `drawSpectrum(spectrum, x, y, width, height, color)`

### Utility Functions

- `getWidth()` - Get window width
- `getHeight()` - Get window height
- `getFPS()` - Get current FPS
- `getAudioSpectrum()` - Get audio frequency spectrum
- `getAudioWaveform()` - Get audio waveform
- `getAudioEnergy()` - Get overall audio energy
- `getAudioBass()` - Get bass energy
- `getAudioMid()` - Get mid-range energy
- `getAudioTreble()` - Get treble energy
- `color(r, g, b, a)` - Create a color
- `colorFromHSV(h, s, v, a)` - Create a color from HSV

## Creating Your Own Visualizations

1. Create a new Lua script in the `scripts` directory
2. Implement the required lifecycle functions
3. Run the visualizer with your script:
   ```
   ./AudioVisualizer scripts/your_script.lua
   ```

## License

MIT License

## Acknowledgments

- SDL2 for window and input handling
- Lua for scripting support
- FFT implementation based on FFTW 
