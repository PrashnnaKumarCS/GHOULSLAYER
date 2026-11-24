#include "ResourceManager.hpp"
#include <iostream>
#include <vector>

ResourceManager::ResourceManager() {
    createFallbackTexture();
    createFallbackFont();
    createFallbackSoundBuffer();
}

void ResourceManager::createFallbackTexture() {
    // Generate a 32x32 magenta/black checkerboard pattern for missing textures
    sf::Image img;
    img.create(32, 32);
    for (unsigned int y = 0; y < 32; ++y) {
        for (unsigned int x = 0; x < 32; ++x) {
            bool isMagenta = ((x / 8) + (y / 8)) % 2 == 0;
            img.setPixel(x, y, isMagenta ? sf::Color(255, 0, 255) : sf::Color(0, 0, 0));
        }
    }
    m_fallbackTexture.loadFromImage(img);
    m_fallbackTexture.setSmooth(false);
}

void ResourceManager::createFallbackFont() {
    // 1. Prefer bundled open-source pixel font (Press Start 2P)
    if (!m_fallbackFont.loadFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        // 2. Fall back to standard Windows monospace system font
        if (!m_fallbackFont.loadFromFile("C:/Windows/Fonts/consola.ttf")) {
            // 3. Fall back to standard Windows Arial
            if (!m_fallbackFont.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                std::cerr << "[ResourceManager INFO] Default system font fallback initialized (empty).\n";
            }
        }
    }
}

void ResourceManager::createFallbackSoundBuffer() {
    // Generate a tiny silent PCM buffer (100 samples at 44100Hz)
    // so attempting to play a missing sound never crashes the audio device
    std::vector<sf::Int16> silentSamples(100, 0);
    m_fallbackSoundBuffer.loadFromSamples(silentSamples.data(), silentSamples.size(), 1, 44100);
}

bool ResourceManager::loadTexture(const std::string& name, const std::string& filepath) {
    sf::Texture texture;
    if (!texture.loadFromFile(filepath)) {
        std::cerr << "[ResourceManager WARNING] Failed to load texture '" << filepath 
                  << "'. Using fallback texture for '" << name << "'.\n";
        return false;
    }
    m_textures[name] = std::move(texture);
    return true;
}

void ResourceManager::insertTexture(const std::string& name, const sf::Texture& texture) {
    m_textures[name] = texture;
}

const sf::Texture& ResourceManager::getTexture(const std::string& name) const {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    std::cerr << "[ResourceManager WARNING] Texture '" << name 
              << "' not found in cache! Returning fallback texture.\n";
    return m_fallbackTexture;
}

bool ResourceManager::hasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

bool ResourceManager::loadFont(const std::string& name, const std::string& filepath) {
    sf::Font font;
    if (!font.loadFromFile(filepath)) {
        std::cerr << "[ResourceManager WARNING] Failed to load font '" << filepath 
                  << "'. Using fallback font for '" << name << "'.\n";
        return false;
    }
    m_fonts[name] = std::move(font);
    return true;
}

const sf::Font& ResourceManager::getFont(const std::string& name) const {
    auto it = m_fonts.find(name);
    if (it != m_fonts.end()) {
        return it->second;
    }
    std::cerr << "[ResourceManager WARNING] Font '" << name 
              << "' not found in cache! Returning fallback font.\n";
    return m_fallbackFont;
}

bool ResourceManager::loadSoundBuffer(const std::string& name, const std::string& filepath) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filepath)) {
        std::cerr << "[ResourceManager WARNING] Failed to load sound buffer '" << filepath 
                  << "'. Using fallback sound for '" << name << "'.\n";
        return false;
    }
    m_soundBuffers[name] = std::move(buffer);
    return true;
}

const sf::SoundBuffer& ResourceManager::getSoundBuffer(const std::string& name) const {
    auto it = m_soundBuffers.find(name);
    if (it != m_soundBuffers.end()) {
        return it->second;
    }
    std::cerr << "[ResourceManager WARNING] SoundBuffer '" << name 
              << "' not found in cache! Returning fallback silent buffer.\n";
    return m_fallbackSoundBuffer;
}
