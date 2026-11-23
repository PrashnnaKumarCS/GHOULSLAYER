#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

// ResourceManager caches textures, fonts, and sound buffers.
// If any asset is missing on disk or requested by a bad name,
// it logs a warning and returns a safe fallback rather than crashing.
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager() = default;

    // Disallow copying to prevent accidental duplicate heavy resource instances
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Texture management
    bool loadTexture(const std::string& name, const std::string& filepath);
    void insertTexture(const std::string& name, const sf::Texture& texture);
    const sf::Texture& getTexture(const std::string& name) const;
    bool hasTexture(const std::string& name) const;

    // Font management
    bool loadFont(const std::string& name, const std::string& filepath);
    const sf::Font& getFont(const std::string& name) const;

    // Sound buffer management
    bool loadSoundBuffer(const std::string& name, const std::string& filepath);
    const sf::SoundBuffer& getSoundBuffer(const std::string& name) const;

private:
    void createFallbackTexture();
    void createFallbackFont();
    void createFallbackSoundBuffer();

    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sf::Font> m_fonts;
    std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;

    // Guaranteed fallbacks so missing assets never cause null reference crashes
    sf::Texture m_fallbackTexture;
    sf::Font m_fallbackFont;
    sf::SoundBuffer m_fallbackSoundBuffer;
};
