#include "Assets.hpp"

#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>

Assets::Assets()
{
}

bool Assets::addTexture(const std::string& textureName, const std::string& path,
                        bool smooth)
{
    sf::Texture texture;

    if (!texture.loadFromFile(path))
    {
        return false;
    }

    texture.setSmooth(smooth);
    m_textureMap[textureName] = texture;
    return true;
}

bool Assets::addFont(const std::string& fontName, const std::string& path)
{
    sf::Font font;

    if (!font.loadFromFile(path))
    {
        return false;
    }

    m_fontMap[fontName] = font;

    return true;
}

bool Assets::addAnimation(const std::string& animationName,
                          const std::string& textureName,
                          const size_t keyframesCount, const size_t duration)
{
    const auto& texture = getTexture(textureName);
    m_animationMap[animationName] = Animation(animationName, texture, keyframesCount,duration);
    return true;
}

const sf::Texture& Assets::getTexture(const std::string& textureName) const
{
    if (m_textureMap.find(textureName) == m_textureMap.end()) {
        std::cerr << "No texture named " << textureName << std::endl;
    }
    return m_textureMap.at(textureName);
}

const Animation& Assets::getAnimation(const std::string& animationName) const
{
    if (m_animationMap.find(animationName) == m_animationMap.end()) {
        std::cerr << "No animation named " << animationName << std::endl;
    }
    return m_animationMap.at(animationName);
}

const sf::Font& Assets::getFont(const std::string& fontName) const
{
    if (m_fontMap.find(fontName) == m_fontMap.end()) {
        std::cerr << "No font named " << fontName << std::endl;
    }
    return m_fontMap.at(fontName);
}

void Assets::loadFromFile(const std::string& filePath)
{
    std::ifstream fin(filePath);
    std::string token;

    while (fin.good())
    {
        fin >> token;
        if (token == "Texture")
        {
            std::string name, path;
            bool smooth;
            fin >> name >> path >> smooth;
            if (!addTexture(name, path , smooth))
            {
                std::cerr << "Failed to add Texture: " << name << " " << path << std::endl;
            }
        }
        else if (token == "Animation")
        {
            std::string name, textureName;
            int keyframesCount, duration;
            fin >> name >> textureName >> keyframesCount >> duration;
            if (!addAnimation(name, textureName, keyframesCount, duration))
            {
                std::cerr << "Failed to add Animation: " << name << " " << std::endl;
            }
        }
        else if (token == "Font")
        {
            std::string name, path;
            fin >> name >> path;
            if (!addFont(name, path))
            {
                std::cerr << "Failed to add Font: " << name << " " << path << std::endl;
            }
        }
    }

    /*
    for (auto [ key , value ] : m_animationMap) {
        std::cerr << key << " " << value.getName() << std::endl;
    }
    */
}