import numpy as np
import pyaudio
import pygame
import pygame.gfxdraw
import sys
from pygame import freetype
import math
import random
import colorsys
from scipy.ndimage import gaussian_filter1d
import time
import ctypes
from ctypes import wintypes
from collections import deque

# Initialize pygame
pygame.init()

# For Windows: Get the screen size properly
if sys.platform == 'win32':
    ctypes.windll.user32.SetProcessDPIAware()
    WIDTH = ctypes.windll.user32.GetSystemMetrics(0)
    HEIGHT = ctypes.windll.user32.GetSystemMetrics(1)
else:
    # For other platforms
    info = pygame.display.Info()
    WIDTH = info.current_w
    HEIGHT = info.current_h

# Create window (windowed borderless mode by default)
WIDTH = 1024
HEIGHT = 768
FLAGS = pygame.NOFRAME  # Borderless window
screen = pygame.display.set_mode((WIDTH, HEIGHT), FLAGS)
pygame.display.set_caption("System Audio Visualizer")
clock = pygame.time.Clock()

# Variables for window dragging and fullscreen toggling
dragging = False
drag_offset_x = 0
drag_offset_y = 0
is_fullscreen = False
window_pos = (100, 100)  # Default window position when not fullscreen
window_size = (WIDTH, HEIGHT)  # Default window size when not fullscreen
last_click_time = 0
double_click_threshold = 300  # milliseconds
monitor_index = 0  # Default to primary monitor

# Set up PyAudio to capture system audio
CHUNK = 1024
FORMAT = pyaudio.paInt16
CHANNELS = 2
RATE = 44100

# Initialize PyAudio
p = pyaudio.PyAudio()

# Find input device that can capture system audio
device_index = None
for i in range(p.get_device_count()):
    device_info = p.get_device_info_by_index(i)
    if 'Stereo Mix' in device_info['name'] or 'What U Hear' in device_info['name'] or 'Wave Out Mix' in device_info['name'] or 'Mix' in device_info['name'] or 'CABLE Output' in device_info['name']:
        device_index = i
        break

# If no system audio device was found, use default input
if device_index is None:
    print("No system audio capture device found. Using default input device.")
    print("This will capture from your microphone instead of your system audio.")
    print("For best results, install a virtual audio cable or enable 'Stereo Mix'.")
    device_index = p.get_default_input_device_info()['index']

# Open stream
try:
    stream = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    input_device_index=device_index,
                    frames_per_buffer=CHUNK)
except OSError:
    print("Error opening audio device. Trying default input device.")
    stream = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    frames_per_buffer=CHUNK)

# Set up frequency analysis
HISTORY_SIZE = 60  # Number of frames to keep in history
frequency_history = deque(maxlen=HISTORY_SIZE)
bass_history = deque(maxlen=HISTORY_SIZE)
mid_history = deque(maxlen=HISTORY_SIZE)
treble_history = deque(maxlen=HISTORY_SIZE)
time_domain_history = deque(maxlen=HISTORY_SIZE)
energy_history = deque(maxlen=HISTORY_SIZE)
transient_history = deque(maxlen=HISTORY_SIZE)

# Precompute some values
center_x = WIDTH // 2
center_y = HEIGHT // 2
frequencies = np.fft.rfftfreq(CHUNK, 1.0/RATE)
max_frequency = 20000  # Maximum frequency to analyze

# Visualization classes
class Particle:
    def __init__(self):
        self.reset()
        
    def reset(self):
        self.x = random.randint(0, WIDTH)
        self.y = random.randint(0, HEIGHT)
        self.size = random.randint(3, 20)  # Larger particles
        self.color = (random.randint(100, 255), random.randint(100, 255), random.randint(100, 255))
        self.speed_x = random.uniform(-3, 3)  # Faster movement
        self.speed_y = random.uniform(-3, 3)  # Faster movement
        self.life = random.randint(50, 200)
        self.rotation = random.uniform(0, 2 * math.pi)
        self.rotation_speed = random.uniform(-0.2, 0.2)
        self.shape_type = random.randint(0, 5)  # More shape variety
        
    def update(self, energy):
        # Move based on audio energy
        self.x += self.speed_x * energy * 25  # Much more responsive
        self.y += self.speed_y * energy * 25  # Much more responsive
        
        # Apply chaotic motion
        self.x += math.sin(time.time() * 2 + self.y * 0.01) * energy * 15
        self.y += math.cos(time.time() * 2 + self.x * 0.01) * energy * 15
        
        # Update rotation
        self.rotation += self.rotation_speed * energy * 5
        
        # Decay life
        self.life -= 1
        
        # Check if particle should be reset
        if self.life <= 0 or self.x < -100 or self.x > WIDTH + 100 or self.y < -100 or self.y > HEIGHT + 100:
            self.reset()
            
    def draw(self, surface):
        # Fade color based on life
        alpha = int(255 * (self.life / 200))
        color = (self.color[0], self.color[1], self.color[2], alpha)
        
        pygame.gfxdraw.filled_circle(surface, int(self.x), int(self.y), self.size, color)
        
        # Add more complex shape drawing based on shape_type
        if self.shape_type == 0:
            # Draw a star shape
            points = []
            for i in range(5):
                angle = self.rotation + i * 2 * math.pi / 5
                # Outer point
                points.append((
                    int(self.x + math.cos(angle) * self.size * 2),
                    int(self.y + math.sin(angle) * self.size * 2)
                ))
                # Inner point
                inner_angle = angle + math.pi / 5
                points.append((
                    int(self.x + math.cos(inner_angle) * self.size * 0.8),
                    int(self.y + math.sin(inner_angle) * self.size * 0.8)
                ))
            if len(points) >= 3:  # Need at least 3 points for a polygon
                pygame.draw.polygon(surface, color, points)
        
        elif self.shape_type == 1:
            # Rotating square
            size = self.size * 2
            points = []
            for i in range(4):
                angle = self.rotation + i * math.pi / 2
                points.append((
                    int(self.x + math.cos(angle) * size),
                    int(self.y + math.sin(angle) * size)
                ))
            pygame.draw.polygon(surface, color, points)
        
        elif self.shape_type == 2:
            # Triangle
            size = self.size * 2
            points = []
            for i in range(3):
                angle = self.rotation + i * 2 * math.pi / 3
                points.append((
                    int(self.x + math.cos(angle) * size),
                    int(self.y + math.sin(angle) * size)
                ))
            pygame.draw.polygon(surface, color, points)
            
        elif self.shape_type == 3:
            # Pulsing circle with rings
            pygame.draw.circle(surface, color, (int(self.x), int(self.y)), 
                              int(self.size * (1 + math.sin(time.time() * 5) * 0.5)), 3)
            pygame.draw.circle(surface, color, (int(self.x), int(self.y)), 
                              int(self.size * 1.5 * (1 + math.sin(time.time() * 5 + 1) * 0.5)), 2)
                              
        elif self.shape_type == 4:
            # Hexagon
            size = self.size * 1.5
            points = []
            for i in range(6):
                angle = self.rotation + i * 2 * math.pi / 6
                points.append((
                    int(self.x + math.cos(angle) * size),
                    int(self.y + math.sin(angle) * size)
                ))
            pygame.draw.polygon(surface, color, points)
        
        elif self.shape_type == 5:
            # Cross shape
            size = self.size * 2
            thickness = max(2, self.size // 2)
            points1 = [
                (int(self.x - thickness), int(self.y - size)),
                (int(self.x + thickness), int(self.y - size)),
                (int(self.x + thickness), int(self.y + size)),
                (int(self.x - thickness), int(self.y + size))
            ]
            points2 = [
                (int(self.x - size), int(self.y - thickness)),
                (int(self.x + size), int(self.y - thickness)),
                (int(self.x + size), int(self.y + thickness)),
                (int(self.x - size), int(self.y + thickness))
            ]
            pygame.draw.polygon(surface, color, points1)
            pygame.draw.polygon(surface, color, points2)

class Fractal:
    def __init__(self, x, y, depth=4):
        self.x = x
        self.y = y
        self.angle = 0
        self.depth = depth
        self.length = 100
        self.hue = 0
        self.branch_factor = random.uniform(0.4, 0.8)
        self.chaos_factor = 0
        
    def update(self, energy, bass, mid, treble):
        # Rotate based on audio energy
        self.angle += energy * 0.1
        
        # Adjust color based on frequency distribution
        self.hue = (self.hue + bass * 0.03) % 1.0
        
        # Adjust size based on mid frequencies
        self.length = 100 + mid * 200
        
        # Adjust complexity based on treble
        self.depth = 2 + int(treble * 6)
        
        # Increase chaos with energy
        self.chaos_factor = energy * 2
        
        # Change branch factor based on mid-frequencies
        self.branch_factor = 0.4 + mid * 0.4
        
    def draw(self, surface, x, y, angle, length, depth):
        if depth <= 0:
            return
            
        # Add chaotic variations to angle and length
        chaos_angle = angle + (random.random() - 0.5) * self.chaos_factor
        chaos_length = length * (1 + (random.random() - 0.5) * self.chaos_factor * 0.2)
        
        # Calculate end point
        end_x = x + math.cos(chaos_angle) * chaos_length
        end_y = y + math.sin(chaos_angle) * chaos_length
        
        # Calculate color
        color = pygame.Color(0, 0, 0)
        # Vary hue based on depth
        depth_hue = (self.hue + depth * 0.1) % 1.0
        rgb = colorsys.hsv_to_rgb(depth_hue, 1.0, 1.0)
        color.r = int(rgb[0] * 255)
        color.g = int(rgb[1] * 255)
        color.b = int(rgb[2] * 255)
        
        # Draw line with thickness based on depth
        thickness = max(1, int(3 * (depth / self.depth) + 1))
        pygame.draw.line(surface, color, (x, y), (end_x, end_y), thickness)
        
        # Draw glowing circle at joint
        glow_radius = int(thickness * 2)
        pygame.draw.circle(surface, color, (int(end_x), int(end_y)), glow_radius)
        
        # Recursively draw branches with varying angles
        new_length = chaos_length * self.branch_factor
        num_branches = 3 + depth % 2  # Vary number of branches 
        
        for i in range(num_branches):
            branch_angle = chaos_angle - math.pi / 4 + (i * math.pi / (num_branches - 1) / 2)
            # Add some randomness to branch angles
            branch_angle += (random.random() - 0.5) * self.chaos_factor * 0.5
            self.draw(surface, end_x, end_y, branch_angle, new_length, depth - 1)

class WaveForm:
    def __init__(self):
        self.points = []
        self.color = (0, 0, 0)
        
    def update(self, waveform_data, energy):
        # Clear points
        self.points = []
        
        # Create points based on waveform
        for i, sample in enumerate(waveform_data):
            x = i * WIDTH // len(waveform_data)
            y = center_y + sample * HEIGHT // 4 * energy
            self.points.append((x, y))
            
        # Update color based on energy
        self.color = pygame.Color(0, 0, 0)
        hue = (time.time() * 0.1) % 1.0
        rgb = colorsys.hsv_to_rgb(hue, 1.0, 1.0)
        self.color = (int(rgb[0] * 255), int(rgb[1] * 255), int(rgb[2] * 255))
        
    def draw(self, surface):
        if len(self.points) < 2:
            return
            
        # Draw smoothed line
        pygame.draw.aalines(surface, self.color, False, self.points, 2)

# Initialize visualization objects
particles = [Particle() for _ in range(200)]
fractal = Fractal(center_x, center_y)
waveform = WaveForm()

# Font for displaying information
pygame.freetype.init()
font = pygame.freetype.SysFont("Arial", 16)

# Main loop
running = True
show_info = True
pause = False
crazy_mode = True  # Start with crazy mode ON by default
extra_crazy_mode = True  # New ultra-crazy mode
rotation_angle = 0
warp_time = 0

# Declare surfaces for rendering
main_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
waveform_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
particle_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
fractal_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
blur_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)

# Function to toggle fullscreen on a specific monitor
def toggle_fullscreen(monitor_idx=0):
    global is_fullscreen, window_pos, window_size, WIDTH, HEIGHT, screen, main_surface, waveform_surface, particle_surface, fractal_surface, blur_surface, center_x, center_y
    
    if is_fullscreen:
        # Return to windowed mode with previous size and position
        WIDTH, HEIGHT = window_size
        screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.NOFRAME)
        pygame.display.set_caption("System Audio Visualizer")
        
        # Set window position
        if sys.platform == 'win32':
            try:
                hwnd = pygame.display.get_wm_info()['window']
                ctypes.windll.user32.SetWindowPos(hwnd, 0, window_pos[0], window_pos[1], 0, 0, 0x0001)
            except Exception as e:
                print(f"Error setting window position: {e}")
    else:
        # Save current window position and size before going fullscreen
        if sys.platform == 'win32':
            try:
                hwnd = pygame.display.get_wm_info()['window']
                rect = wintypes.RECT()
                ctypes.windll.user32.GetWindowRect(hwnd, ctypes.byref(rect))
                window_pos = (rect.left, rect.top)
                window_size = (WIDTH, HEIGHT)
            except Exception as e:
                print(f"Error getting window position: {e}")
                window_pos = (0, 0)
                window_size = (WIDTH, HEIGHT)
        
        # Get monitor info for the selected monitor
        if sys.platform == 'win32':
            try:
                # Get all monitor info
                monitors = []
                def callback(monitor, dc, rect, data):
                    rct = rect.contents
                    monitors.append({
                        'left': rct.left,
                        'top': rct.top,
                        'width': rct.right - rct.left,
                        'height': rct.bottom - rct.top
                    })
                    return 1
                
                callback_type = ctypes.WINFUNCTYPE(ctypes.c_int, ctypes.c_ulong, ctypes.c_ulong, 
                                                ctypes.POINTER(wintypes.RECT), ctypes.c_double)
                callback_func = callback_type(callback)
                ctypes.windll.user32.EnumDisplayMonitors(0, 0, callback_func, 0)
                
                # Use selected monitor (if available)
                if 0 <= monitor_idx < len(monitors):
                    mon = monitors[monitor_idx]
                    WIDTH = mon['width']
                    HEIGHT = mon['height']
                    # Create fullscreen window on the selected monitor
                    screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.NOFRAME)
                    # Move window to the monitor
                    hwnd = pygame.display.get_wm_info()['window']
                    ctypes.windll.user32.SetWindowPos(hwnd, 0, mon['left'], mon['top'], 0, 0, 0x0001)
                else:
                    # Fallback to default monitor
                    WIDTH = ctypes.windll.user32.GetSystemMetrics(0)
                    HEIGHT = ctypes.windll.user32.GetSystemMetrics(1)
                    screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.NOFRAME)
            except Exception as e:
                print(f"Error setting up fullscreen: {e}")
                # Fallback behavior
                WIDTH = 1024
                HEIGHT = 768
                screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.NOFRAME)
        else:
            # Non-Windows platforms
            info = pygame.display.Info()
            WIDTH = info.current_w
            HEIGHT = info.current_h
            screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.NOFRAME)
    
    # Update center values
    center_x = WIDTH // 2
    center_y = HEIGHT // 2
    
    # Recreate surfaces with new dimensions
    main_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
    waveform_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
    particle_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
    fractal_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
    blur_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
    
    # Toggle fullscreen state
    is_fullscreen = not is_fullscreen

while running:
    # Handle events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False
            elif event.key == pygame.K_f:
                # Toggle fullscreen using F key
                toggle_fullscreen()
            elif event.key == pygame.K_i:
                # Toggle info display
                show_info = not show_info
            elif event.key == pygame.K_SPACE:
                # Toggle pause
                pause = not pause
            elif event.key == pygame.K_c:
                # Toggle crazy mode
                crazy_mode = not crazy_mode
            elif event.key == pygame.K_x:
                # Toggle extra crazy mode
                extra_crazy_mode = not extra_crazy_mode
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:  # Left mouse button
                # Check for double click
                current_time = pygame.time.get_ticks()
                if current_time - last_click_time < double_click_threshold:
                    # Double click - toggle fullscreen on the monitor where the click happened
                    if sys.platform == 'win32':
                        try:
                            # Determine which monitor was clicked
                            mouse_x, mouse_y = event.pos
                            if not is_fullscreen:
                                # Add window position offset if in windowed mode
                                hwnd = pygame.display.get_wm_info()['window']
                                rect = wintypes.RECT()
                                ctypes.windll.user32.GetWindowRect(hwnd, ctypes.byref(rect))
                                mouse_x += rect.left
                                mouse_y += rect.top
                            
                            # Get all monitor info
                            monitors = []
                            def callback(monitor, dc, rect, data):
                                rct = rect.contents
                                monitors.append({
                                    'left': rct.left,
                                    'top': rct.top,
                                    'right': rct.right,
                                    'bottom': rct.bottom
                                })
                                return 1
                            
                            callback_type = ctypes.WINFUNCTYPE(ctypes.c_int, ctypes.c_ulong, ctypes.c_ulong, 
                                                              ctypes.POINTER(wintypes.RECT), ctypes.c_double)
                            callback_func = callback_type(callback)
                            ctypes.windll.user32.EnumDisplayMonitors(0, 0, callback_func, 0)
                            
                            # Find which monitor contains the mouse
                            for i, mon in enumerate(monitors):
                                if (mon['left'] <= mouse_x < mon['right'] and 
                                    mon['top'] <= mouse_y < mon['bottom']):
                                    monitor_index = i
                                    break
                        except Exception as e:
                            print(f"Error detecting monitor: {e}")
                    
                    # Toggle fullscreen on the selected monitor
                    toggle_fullscreen(monitor_index)
                else:
                    # Start dragging
                    if not is_fullscreen:
                        dragging = True
                        mouse_x, mouse_y = event.pos
                        drag_offset_x = mouse_x
                        drag_offset_y = mouse_y
                
                # Update last click time
                last_click_time = current_time
        
        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:  # Left mouse button
                dragging = False
        
        elif event.type == pygame.MOUSEMOTION:
            if dragging and not is_fullscreen:
                try:
                    # Move the window when dragging
                    mouse_x, mouse_y = event.pos
                    dx = mouse_x - drag_offset_x
                    dy = mouse_y - drag_offset_y
                    
                    if sys.platform == 'win32':
                        # Get current window position
                        hwnd = pygame.display.get_wm_info()['window']
                        rect = wintypes.RECT()
                        ctypes.windll.user32.GetWindowRect(hwnd, ctypes.byref(rect))
                        
                        # Move the window
                        new_x = rect.left + dx
                        new_y = rect.top + dy
                        ctypes.windll.user32.SetWindowPos(hwnd, 0, new_x, new_y, 0, 0, 0x0001)
                        
                        # Update window position
                        window_pos = (new_x, new_y)
                except Exception as e:
                    print(f"Error dragging window: {e}")
                    dragging = False
    
    if not pause:
        # Read audio data
        try:
            raw_data = np.frombuffer(stream.read(CHUNK, exception_on_overflow=False), dtype=np.int16)
        except OSError:
            # Handle audio device errors
            print("Audio device error. Trying to reconnect...")
            try:
                stream.close()
                stream = p.open(format=FORMAT,
                                channels=CHANNELS,
                                rate=RATE,
                                input=True,
                                input_device_index=device_index,
                                frames_per_buffer=CHUNK)
                continue
            except:
                print("Failed to reconnect. Using random data.")
                raw_data = np.random.randint(-32768, 32767, CHUNK * CHANNELS, dtype=np.int16)
        
        # Normalize data to -1.0 to 1.0
        data = raw_data.astype(np.float32) / 32768.0
        
        # Separate channels
        if CHANNELS == 2:
            left_channel = data[0::2]
            right_channel = data[1::2]
            mono_data = (left_channel + right_channel) / 2
        else:
            mono_data = data
        
        # Apply window function
        window = np.hanning(len(mono_data))
        windowed_data = mono_data * window
        
        # Compute FFT
        spectrum = np.abs(np.fft.rfft(windowed_data))
        
        # Calculate frequency bands
        bass_idx = np.where(frequencies < 250)[0]
        mid_idx = np.where((frequencies >= 250) & (frequencies < 2000))[0]
        treble_idx = np.where((frequencies >= 2000) & (frequencies < max_frequency))[0]
        
        bass = np.mean(spectrum[bass_idx]) if len(bass_idx) > 0 else 0
        mid = np.mean(spectrum[mid_idx]) if len(mid_idx) > 0 else 0
        treble = np.mean(spectrum[treble_idx]) if len(treble_idx) > 0 else 0
        
        # Normalize
        bass = np.clip(bass / 5.0, 0, 1)
        mid = np.clip(mid / 5.0, 0, 1)
        treble = np.clip(treble / 5.0, 0, 1)
        
        # Calculate total energy
        energy = (bass + mid + treble) / 3.0
        
        # Detect transients (sudden changes in energy)
        transient = 0
        if len(energy_history) > 0:
            transient = max(0, energy - np.mean(energy_history)) * 5
        
        # Add to history
        frequency_history.append(spectrum)
        bass_history.append(bass)
        mid_history.append(mid)
        treble_history.append(treble)
        time_domain_history.append(mono_data)
        energy_history.append(energy)
        transient_history.append(transient)
        
        # Update rotation angle
        rotation_angle += energy * 0.02
        warp_time += 0.01 + energy * 0.1  # Update warp time
        
        # Clear surfaces with fade effect for trails
        main_surface.fill((0, 0, 0, 10))  # Nearly transparent black for trails
        waveform_surface.fill((0, 0, 0, 0))
        particle_surface.fill((0, 0, 0, 10))  # Slightly transparent for particle trails
        fractal_surface.fill((0, 0, 0, 0))
        blur_surface.fill((0, 0, 0, 0))
        
        # Update waveform
        waveform.update(mono_data, energy)
        waveform.draw(waveform_surface)
        
        # Update particles
        for particle in particles:
            particle.update(energy)
            particle.draw(particle_surface)
        
        # Update fractal
        fractal.update(energy, bass, mid, treble)
        fractal.draw(fractal_surface, center_x, center_y, rotation_angle, fractal.length, fractal.depth)
        
        # Apply visual effects based on crazy_mode
        if crazy_mode:
            # Apply kaleidoscope effect
            kaleidoscope_size = int(min(WIDTH, HEIGHT) * 0.8)
            kaleidoscope_surface = pygame.Surface((kaleidoscope_size, kaleidoscope_size), pygame.SRCALPHA)
            
            # Create a kaleidoscope from the fractal and particle surfaces
            segments = 8 + int(energy * 8)  # More segments based on energy
            segment_angle = 2 * math.pi / segments
            
            for i in range(segments):
                angle = i * segment_angle + rotation_angle
                segment_surface = pygame.Surface((kaleidoscope_size // 2, kaleidoscope_size // 2), pygame.SRCALPHA)
                
                # Copy a quarter of the fractal surface
                segment_surface.blit(fractal_surface, (0, 0), (center_x, center_y, kaleidoscope_size // 2, kaleidoscope_size // 2))
                
                # Rotate and position the segment
                rotated_segment = pygame.transform.rotate(segment_surface, math.degrees(angle))
                rot_rect = rotated_segment.get_rect(center=(kaleidoscope_size // 2, kaleidoscope_size // 2))
                
                # Draw to kaleidoscope surface
                kaleidoscope_surface.blit(rotated_segment, rot_rect)
            
            # Apply ripple effect (simplified version that avoids pixelcopy issues)
            ripple_surface = pygame.Surface((kaleidoscope_size, kaleidoscope_size), pygame.SRCALPHA)

            for radius in range(0, kaleidoscope_size//2, 10):  # More dense ripples
                # Calculate ripple intensity based on audio
                ripple_intensity = 30 * energy * math.sin(radius/10 - warp_time * 5)
                
                # Color based on radius and time
                hue = (radius / kaleidoscope_size + warp_time * 0.1) % 1.0
                r, g, b = [int(c * 255) for c in colorsys.hsv_to_rgb(hue, 0.8, 1.0)]
                ripple_color = (r, g, b, 100)  # More visible ripples
                
                # Draw ripple circle
                pygame.draw.circle(ripple_surface, ripple_color, 
                                (kaleidoscope_size//2, kaleidoscope_size//2), 
                                radius + int(ripple_intensity), 
                                3)  # Thicker ripples

            # Apply the ripple effect with blending
            kaleidoscope_surface.blit(ripple_surface, (0, 0), special_flags=pygame.BLEND_ADD)
            
            if extra_crazy_mode:
                # Apply extreme warping
                warp_surface = pygame.Surface((kaleidoscope_size, kaleidoscope_size), pygame.SRCALPHA)
                
                # Create space-time distortion patterns
                for i in range(20):  # More distortion patterns
                    # Wild distortion shapes
                    distort_time = warp_time * (0.5 + i * 0.1)
                    x1 = kaleidoscope_size//2 + math.sin(distort_time * 1.1) * kaleidoscope_size//3 * energy
                    y1 = kaleidoscope_size//2 + math.cos(distort_time * 0.7) * kaleidoscope_size//3 * energy
                    
                    x2 = kaleidoscope_size//2 + math.sin(distort_time * 1.3 + 2) * kaleidoscope_size//3 * energy
                    y2 = kaleidoscope_size//2 + math.cos(distort_time * 0.9 + 2) * kaleidoscope_size//3 * energy
                    
                    # Size and shape vary with audio and time
                    radius = (20 + math.sin(distort_time) * 10) * (0.5 + energy * 1.5)
                    thickness = int(3 + energy * 5)
                    
                    # Color based on time and frequency
                    c_hue = (distort_time * 0.1 + i / 20) % 1.0
                    
                    # Select colors based on frequency bands
                    if i % 3 == 0:  # Bass-reactive
                        color_intensity = bass
                        c_hue = (c_hue + bass * 0.2) % 1.0
                    elif i % 3 == 1:  # Mid-reactive
                        color_intensity = mid
                        c_hue = (c_hue + mid * 0.2) % 1.0
                    else:  # Treble-reactive
                        color_intensity = treble
                        c_hue = (c_hue + treble * 0.2) % 1.0
                    
                    r, g, b = [int(c * 255) for c in colorsys.hsv_to_rgb(c_hue, 0.9, 0.8 + color_intensity * 0.2)]
                    color = (r, g, b, 180)
                    
                    # Draw various distortion elements
                    if i % 5 == 0:
                        # Glowing circles
                        pygame.draw.circle(warp_surface, color, (int(x1), int(y1)), int(radius), 0)
                    elif i % 5 == 1:
                        # Connecting lines
                        pygame.draw.line(warp_surface, color, (int(x1), int(y1)), (int(x2), int(y2)), thickness)
                    elif i % 5 == 2:
                        # Bezier-like curves
                        points = []
                        steps = 20
                        for t in range(steps+1):
                            t_val = t / steps
                            cx = x1 * (1-t_val) + x2 * t_val + math.sin(t_val * math.pi * 2 + distort_time) * 50 * energy
                            cy = y1 * (1-t_val) + y2 * t_val + math.cos(t_val * math.pi * 2 + distort_time) * 50 * energy
                            points.append((int(cx), int(cy)))
                        
                        if len(points) > 1:
                            pygame.draw.lines(warp_surface, color, False, points, thickness)
                    elif i % 5 == 3:
                        # Spiral
                        spiral_points = []
                        spiral_steps = 30
                        for t in range(spiral_steps):
                            spiral_radius = t / spiral_steps * radius
                            spiral_angle = t / spiral_steps * 10 * math.pi + distort_time
                            sx = x1 + math.cos(spiral_angle) * spiral_radius
                            sy = y1 + math.sin(spiral_angle) * spiral_radius
                            spiral_points.append((int(sx), int(sy)))
                        
                        if len(spiral_points) > 1:
                            pygame.draw.lines(warp_surface, color, False, spiral_points, thickness)
                    else:
                        # Random polygon
                        poly_points = []
                        poly_sides = 5 + i % 4
                        for p in range(poly_sides):
                            poly_angle = p / poly_sides * 2 * math.pi + distort_time
                            px = x1 + math.cos(poly_angle) * radius
                            py = y1 + math.sin(poly_angle) * radius
                            poly_points.append((int(px), int(py)))
                        
                        if len(poly_points) >= 3:
                            pygame.draw.polygon(warp_surface, color, poly_points, thickness)
                
                # Blend the warp surface with the kaleidoscope
                kaleidoscope_surface.blit(warp_surface, (0, 0), special_flags=pygame.BLEND_ADD)
                
            # Position the kaleidoscope
            kaleidoscope_rect = kaleidoscope_surface.get_rect(center=(center_x, center_y))
            main_surface.blit(kaleidoscope_surface, kaleidoscope_rect, special_flags=pygame.BLEND_ADD)
            
        # Draw frequency visualization rings
        if len(frequency_history) > 0:
            # Draw circular frequency visualization
            freq_radius = int(min(WIDTH, HEIGHT) * 0.4)
            freq_surface = pygame.Surface((freq_radius * 2, freq_radius * 2), pygame.SRCALPHA)
            
            # Get latest spectrum
            latest_spectrum = frequency_history[-1]
            
            # Draw frequency bins as circular bars
            for i, magnitude in enumerate(latest_spectrum[:100]):  # Limit to first 100 frequency bins
                # Normalize and scale magnitude
                mag = min(1.0, magnitude / 5.0)
                
                # Calculate angle
                angle = i / 100 * 2 * math.pi
                
                # Calculate color based on frequency
                hue = i / 100
                rgb = colorsys.hsv_to_rgb(hue, 1.0, 1.0)
                color = (int(rgb[0] * 255), int(rgb[1] * 255), int(rgb[2] * 255), 200)
                
                # Draw bar
                inner_radius = freq_radius * 0.2
                outer_radius = inner_radius + freq_radius * 0.8 * mag
                
                start_pos = (
                    freq_radius + int(math.cos(angle) * inner_radius),
                    freq_radius + int(math.sin(angle) * inner_radius)
                )
                end_pos = (
                    freq_radius + int(math.cos(angle) * outer_radius),
                    freq_radius + int(math.sin(angle) * outer_radius)
                )
                
                pygame.draw.line(freq_surface, color, start_pos, end_pos, 3)
            
            # Position the frequency visualization
            freq_rect = freq_surface.get_rect(center=(center_x, center_y))
            main_surface.blit(freq_surface, freq_rect)

        # Draw waveform
        waveform_scaled = pygame.transform.scale(waveform_surface, (WIDTH, int(HEIGHT * (0.5 + energy * 0.5))))
        waveform_rect = waveform_scaled.get_rect(center=(center_x, center_y))
        main_surface.blit(waveform_scaled, waveform_rect, special_flags=pygame.BLEND_ADD)
        
        # Draw particles
        main_surface.blit(particle_surface, (0, 0), special_flags=pygame.BLEND_ADD)
        
        # Apply bass-reactive shockwave
        if bass > 0.7:  # More frequent shockwaves
            # Create a shockwave effect
            shockwave_radius = int(bass * 500)  # Bigger shockwaves
            shockwave_thickness = int(10 + bass * 30)  # Thicker rings
            shockwave_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
            
            # Multi-colored, multi-ring shockwave
            for i in range(3):
                hue = (warp_time * 0.2 + i * 0.3) % 1.0
                r, g, b = [int(c * 255) for c in colorsys.hsv_to_rgb(hue, 0.9, 1.0)]
                pygame.draw.circle(shockwave_surface, (r, g, b, 150), 
                                  (center_x, center_y), 
                                  shockwave_radius - i * 50, 
                                  shockwave_thickness)
                
            main_surface.blit(shockwave_surface, (0, 0), special_flags=pygame.BLEND_ADD)
        
        # Add mid-frequency reactive lightning bolts
        if mid > 0.75:
            lightning_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
            
            # Generate lightning branches
            for _ in range(int(mid * 5)):
                start_x = random.randint(0, WIDTH)
                start_y = 0
                points = [(start_x, start_y)]
                
                # Create a jagged path
                current_x = start_x
                current_y = start_y
                
                segments = 10 + int(mid * 20)
                for i in range(segments):
                    # More horizontal near top, more vertical near bottom
                    progress = i / segments
                    horizontal_factor = 1.0 - progress
                    
                    # Add a new point with random offset
                    offset_x = random.randint(-80, 80) * horizontal_factor * mid
                    offset_y = random.randint(10, 30) * mid
                    
                    current_x += offset_x
                    current_y += offset_y
                    
                    # Keep within bounds
                    current_x = max(0, min(WIDTH, current_x))
                    current_y = max(0, min(HEIGHT, current_y))
                    
                    points.append((current_x, current_y))
                
                # Draw the main lightning bolt
                if len(points) > 1:
                    # Color based on position and time
                    hue = (start_x / WIDTH + warp_time * 0.1) % 1.0
                    r, g, b = [int(c * 255) for c in colorsys.hsv_to_rgb(hue, 0.7, 1.0)]
                    pygame.draw.lines(lightning_surface, (r, g, b, 200), False, points, int(3 + mid * 5))
                    
                    # Add glow effect
                    for i in range(3):
                        glow_thickness = 6 - i * 2
                        glow_alpha = 100 - i * 30
                        pygame.draw.lines(lightning_surface, (r, g, b, glow_alpha), 
                                         False, points, int(glow_thickness + mid * 6))
            
            main_surface.blit(lightning_surface, (0, 0), special_flags=pygame.BLEND_ADD)
        
        # Add treble-reactive particles explosion
        if treble > 0.8:
            explosion_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
            
            # Source of explosion
            ex_x = center_x + random.randint(-200, 200)
            ex_y = center_y + random.randint(-200, 200)
            
            # Number of particles based on treble level
            num_particles = int(treble * 100)
            
            for _ in range(num_particles):
                # Random angle and distance
                angle = random.random() * math.pi * 2
                distance = random.random() * 100 * treble
                
                # Calculate position
                p_x = ex_x + math.cos(angle) * distance
                p_y = ex_y + math.sin(angle) * distance
                
                # Particle size and color
                p_size = random.randint(2, 10)
                hue = (warp_time + distance / 100) % 1.0
                r, g, b = [int(c * 255) for c in colorsys.hsv_to_rgb(hue, 0.8, 1.0)]
                
                # Draw particle
                pygame.draw.circle(explosion_surface, (r, g, b, 200), (int(p_x), int(p_y)), p_size)
                
                # Add connecting lines for some particles
                if random.random() < 0.3:
                    line_end_x = p_x + (p_x - ex_x) * 0.5
                    line_end_y = p_y + (p_y - ex_y) * 0.5
                    pygame.draw.line(explosion_surface, (r, g, b, 100), 
                                    (int(p_x), int(p_y)), 
                                    (int(line_end_x), int(line_end_y)), 
                                    max(1, int(p_size/2)))
            
            main_surface.blit(explosion_surface, (0, 0), special_flags=pygame.BLEND_ADD)
            
        # Add energy-reactive tunnel effect
        if extra_crazy_mode and energy > 0.5:
            tunnel_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
            
            # Draw concentric shapes that create a tunnel illusion
            num_rings = 15
            max_radius = (WIDTH + HEIGHT) // 2
            
            for i in range(num_rings):
                # Calculate ring properties
                progress = i / num_rings
                radius = max_radius * progress
                
                # Distort radius based on audio and time
                radius_distort = 1.0 + math.sin(warp_time * 2 + progress * math.pi * 4) * 0.2 * energy
                radius *= radius_distort
                
                # Offset center based on audio
                offset_x = math.sin(warp_time + progress * math.pi) * 100 * energy
                offset_y = math.cos(warp_time * 1.3 + progress * math.pi) * 100 * energy
                
                # Color based on depth
                hue = (progress + warp_time * 0.1) % 1.0
                
                # Vary color based on frequency bands
                if i % 3 == 0:  # Bass-responsive rings
                    hue = (hue + bass * 0.2) % 1.0
                    saturation = 0.7 + bass * 0.3
                    value = 0.6 + bass * 0.4
                elif i % 3 == 1:  # Mid-responsive rings
                    hue = (hue + mid * 0.2) % 1.0
                    saturation = 0.7 + mid * 0.3
                    value = 0.6 + mid * 0.4
                else:  # Treble-responsive rings
                    hue = (hue + treble * 0.2) % 1.0
                    saturation = 0.7 + treble * 0.3
                    value = 0.6 + treble * 0.4
                
                r, g, b = [int(c * 255) for c in colorsys.hsv_to_rgb(hue, saturation, value)]
                
                # Alternate between different shapes
                shape_type = (i + int(warp_time * 2)) % 4
                
                if shape_type == 0:
                    # Circle
                    pygame.draw.circle(tunnel_surface, (r, g, b, 150), 
                                      (center_x + int(offset_x), center_y + int(offset_y)), 
                                      int(radius), 
                                      max(1, int(10 * (1 - progress) * energy)))
                elif shape_type == 1:
                    # Square
                    size = int(radius * 1.8)  # Make square comparable to circle
                    rect = pygame.Rect(
                        center_x + int(offset_x) - size//2,
                        center_y + int(offset_y) - size//2,
                        size, size
                    )
                    pygame.draw.rect(tunnel_surface, (r, g, b, 150), rect, 
                                    max(1, int(10 * (1 - progress) * energy)))
                elif shape_type == 2:
                    # Star
                    points = []
                    num_points = 5
                    inner_radius = radius * 0.5
                    
                    for j in range(num_points * 2):
                        # Alternate between outer and inner points
                        curr_radius = radius if j % 2 == 0 else inner_radius
                        angle = j * math.pi / num_points + warp_time
                        
                        point_x = center_x + int(offset_x) + int(math.cos(angle) * curr_radius)
                        point_y = center_y + int(offset_y) + int(math.sin(angle) * curr_radius)
                        points.append((point_x, point_y))
                    
                    if len(points) >= 3:
                        pygame.draw.polygon(tunnel_surface, (r, g, b, 150), points, 
                                          max(1, int(8 * (1 - progress) * energy)))
                else:
                    # Triangle
                    points = []
                    for j in range(3):
                        angle = j * (2 * math.pi / 3) + warp_time
                        point_x = center_x + int(offset_x) + int(math.cos(angle) * radius)
                        point_y = center_y + int(offset_y) + int(math.sin(angle) * radius)
                        points.append((point_x, point_y))
                    
                    pygame.draw.polygon(tunnel_surface, (r, g, b, 150), points, 
                                      max(1, int(8 * (1 - progress) * energy)))
            
            main_surface.blit(tunnel_surface, (0, 0), special_flags=pygame.BLEND_ADD)
        
        # Apply time-based color shift with more intensity
        color_shift_surface = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
        hue = (warp_time * 0.05) % 1.0
        rgb = colorsys.hsv_to_rgb(hue, 0.4, 0.8)  # More saturation
        color_shift_surface.fill((int(rgb[0] * 255), int(rgb[1] * 255), int(rgb[2] * 255), 40))  # More visible
        main_surface.blit(color_shift_surface, (0, 0), special_flags=pygame.BLEND_MULT)
        
        # Draw to screen
        screen.fill((0, 0, 0))
        screen.blit(main_surface, (0, 0))
        
        # Draw info text if enabled
        if show_info:
            info_text = [
                f"FPS: {clock.get_fps():.1f}",
                f"Bass: {bass:.2f}",
                f"Mid: {mid:.2f}",
                f"Treble: {treble:.2f}",
                f"Energy: {energy:.2f}",
                f"Transient: {transient:.2f}",
                f"Crazy Mode: {'ON' if crazy_mode else 'OFF'}",
                f"EXTRA Crazy Mode: {'ON' if extra_crazy_mode else 'OFF'}",
                "Press: ESC to quit, F for fullscreen,",
                "SPACE to pause, I to toggle info,", 
                "C to toggle crazy mode, X to toggle EXTRA crazy mode"
            ]
            
            y_offset = 10
            for text in info_text:
                text_surface, text_rect = font.render(text, (255, 255, 255))
                screen.blit(text_surface, (10, y_offset))
                y_offset += text_rect.height + 5
    
    # Update the display
    pygame.display.flip()
    
    # Cap the frame rate
    clock.tick(60)

# Clean up
stream.stop_stream()
stream.close()
p.terminate()
pygame.quit()
sys.exit()