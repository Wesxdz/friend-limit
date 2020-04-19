#pragma once

#include <map>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class Resources
{
public:
    Resources();
    ~Resources();
    static Resources* inst;
    
private:
	sf::String resourcePath;
	sf::String texturesFile;
	sf::String soundsFile;
	sf::String fontsFile;
	sf::String shadersFile;

public:
	// TODO: Make this resource manager generic enough to manage any type
	std::map<sf::String, sf::Texture> textures;
	std::map<sf::String, sf::SoundBuffer> sounds;
	std::map<sf::String, sf::Font> fonts;
	std::map<sf::String, sf::Shader*> shaders;
    
    sf::Texture* LoadTexture(sf::String asset);
	sf::Font* LoadFont(sf::String asset);
	sf::SoundBuffer* LoadSoundBuffer(sf::String asset);
	sf::Shader* LoadShader(sf::String asset);
};
