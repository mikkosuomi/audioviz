# RetroWave Oscilloscope Visualizer

This is a new visualization for AudioVisualizer that displays audio waveforms in a 1980s retro cyberpunk/synthwave style.

## Features

- Beautiful retrowave aesthetic with purple/pink gradient sky
- Mountains in the background
- Stylized sun with grid overlay
- 3D-perspective grid floor
- Audio-reactive waveform display
- Starry sky with twinkling stars
- All elements respond to audio input
- Sound-reactive glowing effects

## How It Works

The RetroWave Oscilloscope visualizer creates a synthwave-inspired landscape with mountains, a setting sun, and a grid extending to the horizon. The audio waveform is displayed prominently in the foreground, with the amplification factor controlling how much the waveform is magnified.

The bass frequencies influence the sun pulsing and horizon effects, mid frequencies affect the color intensity and grid brightness, and treble frequencies control the stars and fine details.

## Usage

1. Build the project with the updated CMakeLists.txt:
   ```
   cd AudioVisualizer
   ./build.bat
   ```

2. Run the AudioVisualizer application:
   ```
   ./build/bin/Release/AudioVisualizer.exe
   ```

3. Use the visualizer controls to switch to the "RetroWave Oscilloscope" visualization.

4. Adjust the amplification factor to increase or decrease the sensitivity of the visualization to audio input.

## Technical Details

The visualization is implemented in:
- AudioVisualizer/include/visualizations/RetroWaveOscilloscopeVisualizer.h
- AudioVisualizer/src/visualizations/RetroWaveOscilloscopeVisualizer.cpp

It includes the following render components:
- Background gradient sky
- Twinkling stars
- Setting sun with grid pattern
- Mountain ranges in the background
- Horizon line with glow effect
- 3D grid floor with perspective
- Audio waveform display
- Reflection effects on the grid floor

## License

This visualizer is provided under the same license as the rest of the AudioVisualizer project. 