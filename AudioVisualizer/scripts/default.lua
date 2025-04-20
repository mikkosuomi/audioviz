--[[
    Default Audio Visualization Script
    
    This script demonstrates basic visualization capabilities using Lua.
]]

-- Global state
local time = 0
local particles = {}
local maxParticles = 200
local warpTime = 0
local rotationAngle = 0

-- Initialize the visualization
function onInit()
    print("Initializing default visualization...")
    
    -- Create initial particles
    for i = 1, maxParticles do
        particles[i] = createParticle()
    end
    
    return true
end

-- Update visualization state
function onUpdate(deltaTime, audio)
    time = time + deltaTime
    warpTime = warpTime + deltaTime * (0.1 + audio.energy * 0.2)
    rotationAngle = rotationAngle + deltaTime * audio.energy * 0.5
    
    -- Update particles based on audio
    for i, particle in ipairs(particles) do
        updateParticle(particle, audio, deltaTime)
        
        -- Respawn dead particles
        if particle.life <= 0 then
            particles[i] = createParticle()
        end
    end
    
    return true
end

-- Render the visualization
function onRender(renderer)
    -- Clear with a dark background
    renderer:drawFilledRect(0, 0, getWidth(), getHeight(), color(0.05, 0.05, 0.1, 1.0))
    
    -- Draw audio spectrum
    drawSpectrum(renderer)
    
    -- Draw particles
    for _, particle in ipairs(particles) do
        drawParticle(renderer, particle)
    end
    
    -- Draw waveform
    drawWaveform(renderer)
    
    -- Apply special effects
    if getAudioEnergy() > 0.6 then
        -- Add bloom/glow when audio is energetic
        renderer:applyBlur(getAudioEnergy() * 0.5)
    end
    
    -- Apply color shift based on time
    local hue = (warpTime * 0.1) % 1.0
    renderer:applyColorShift(colorFromHSV(hue, 0.3, 0.7, 0.2))
    
    -- Draw info text
    drawInfo(renderer)
    
    return true
end

-- Cleanup resources
function onShutdown()
    print("Shutting down visualization...")
    return true
end

------------------
-- Helper functions
------------------

function createParticle()
    local particle = {}
    particle.x = math.random(0, getWidth())
    particle.y = math.random(0, getHeight())
    particle.size = math.random(3, 20)
    particle.speedX = math.random(-2, 2)
    particle.speedY = math.random(-2, 2)
    particle.life = math.random(50, 200)
    particle.maxLife = particle.life
    particle.rotation = math.random() * math.pi * 2
    particle.rotationSpeed = (math.random() - 0.5) * 0.2
    particle.hue = math.random()
    particle.shapeType = math.random(0, 5)
    
    return particle
end

function updateParticle(particle, audio, deltaTime)
    -- Move based on speed and audio
    particle.x = particle.x + particle.speedX * audio.energy * 25 * deltaTime
    particle.y = particle.y + particle.speedY * audio.energy * 25 * deltaTime
    
    -- Apply chaotic motion
    particle.x = particle.x + math.sin(time * 2 + particle.y * 0.01) * audio.energy * 15 * deltaTime
    particle.y = particle.y + math.cos(time * 2 + particle.x * 0.01) * audio.energy * 15 * deltaTime
    
    -- Update rotation
    particle.rotation = particle.rotation + particle.rotationSpeed * audio.energy * 5 * deltaTime
    
    -- Update color based on audio frequencies
    if particle.shapeType % 3 == 0 then
        -- Bass reactive
        particle.hue = (particle.hue + audio.bass * 0.01) % 1.0
    elseif particle.shapeType % 3 == 1 then
        -- Mid reactive
        particle.hue = (particle.hue + audio.mid * 0.01) % 1.0
    else
        -- Treble reactive
        particle.hue = (particle.hue + audio.treble * 0.01) % 1.0
    end
    
    -- Decay life
    particle.life = particle.life - 1
    
    -- Wrap around screen edges
    if particle.x < -50 then particle.x = getWidth() + 50 end
    if particle.x > getWidth() + 50 then particle.x = -50 end
    if particle.y < -50 then particle.y = getHeight() + 50 end
    if particle.y > getHeight() + 50 then particle.y = -50 end
end

function drawParticle(renderer, particle)
    -- Calculate alpha based on life
    local alpha = particle.life / particle.maxLife
    
    -- Get color from HSV
    local particleColor = colorFromHSV(particle.hue, 1.0, 1.0, alpha)
    
    -- Draw particle based on shape type
    renderer:drawParticle(particle.x, particle.y, particle.size, particleColor, particle.shapeType)
end

function drawSpectrum(renderer)
    local spectrum = getAudioSpectrum()
    local width = getWidth()
    local height = getHeight()
    local centerX = width / 2
    local centerY = height / 2
    
    -- Draw circular spectrum visualization
    local spectrumRadius = math.min(width, height) * 0.4
    
    for i = 1, #spectrum do
        local angle = (i / #spectrum) * math.pi * 2
        local magnitude = spectrum[i] * 0.8
        
        -- Calculate inner and outer points of the spectrum bar
        local innerRadius = spectrumRadius * 0.5
        local outerRadius = innerRadius + spectrumRadius * 0.5 * magnitude
        
        local x1 = centerX + math.cos(angle) * innerRadius
        local y1 = centerY + math.sin(angle) * innerRadius
        local x2 = centerX + math.cos(angle) * outerRadius
        local y2 = centerY + math.sin(angle) * outerRadius
        
        -- Color based on frequency
        local hue = i / #spectrum
        local spectrumColor = colorFromHSV(hue, 1.0, 1.0, 0.8)
        
        -- Draw the line
        renderer:drawLine(x1, y1, x2, y2, spectrumColor, 2.0)
    end
end

function drawWaveform(renderer)
    local waveform = getAudioWaveform()
    local width = getWidth()
    local height = getHeight()
    local centerY = height / 2
    
    -- Draw waveform at bottom of screen
    local waveformHeight = height * 0.2
    local waveformTop = height - waveformHeight - 20
    
    -- Create waveform color based on time
    local waveformHue = (time * 0.1) % 1.0
    local waveformColor = colorFromHSV(waveformHue, 1.0, 1.0, 0.8)
    
    -- Draw waveform
    renderer:drawWaveform(waveform, 0, waveformTop, width, waveformHeight, waveformColor)
end

function drawInfo(renderer)
    local textColor = color(1.0, 1.0, 1.0, 0.8)
    local bgColor = color(0.0, 0.0, 0.0, 0.5)
    local padding = 10
    
    -- Background for text
    renderer:drawFilledRect(padding, padding, 200, 120, bgColor)
    
    -- Draw info text
    drawText(renderer, "Audio Visualizer", padding + 10, padding + 10, textColor)
    drawText(renderer, "FPS: " .. getFPS(), padding + 10, padding + 30, textColor)
    drawText(renderer, "Energy: " .. string.format("%.2f", getAudioEnergy()), padding + 10, padding + 50, textColor)
    drawText(renderer, "Bass: " .. string.format("%.2f", getAudioBass()), padding + 10, padding + 70, textColor)
    drawText(renderer, "Mid: " .. string.format("%.2f", getAudioMid()), padding + 10, padding + 90, textColor)
    drawText(renderer, "Treble: " .. string.format("%.2f", getAudioTreble()), padding + 10, padding + 110, textColor)
end 