#include "resources.h"

Resources* Resources::inst;

Resources::Resources() :
	resourcePath{ "../resources" },
	texturesFile{ "images" },
	soundsFile	{ "sounds" },
	fontsFile	{ "fonts" },
	shadersFile { "shaders" } {}

Resources::~Resources()
{
	for (auto pair : shaders) {
		delete pair.second;
	}
}

sf::Texture* Resources::LoadTexture(sf::String asset)
{
    if (textures.count(asset)) {
        return &textures[asset];
    }
    sf::Texture texture;
    texture.loadFromFile(resourcePath + "/" + texturesFile + "/" + asset);
    textures[asset] = texture;
    return &textures[asset];
}

sf::Font* Resources::LoadFont(sf::String asset)
{
    if (fonts.count(asset)) {
        return &fonts[asset];
    }
    sf::Font font;
    font.loadFromFile(resourcePath + "/" + fontsFile + "/" + asset);
    fonts[asset] = font;
    return &fonts[asset];
}

sf::SoundBuffer* Resources::LoadSoundBuffer(sf::String asset)
{
    if (sounds.count(asset)) {
        return &sounds[asset];
    }
    sf::SoundBuffer sound;
    sound.loadFromFile(resourcePath + "/" + soundsFile + "/" + asset);
    sounds[asset] = sound;
    return &sounds[asset];
}

sf::Shader* Resources::LoadShader(sf::String asset)
{
    if (shaders.count(asset)) {
        return shaders[asset];
    }
    sf::Shader* shader = new sf::Shader;
    if (!shader->loadFromFile(resourcePath + "/" + shadersFile + "/" + asset + ".vert", resourcePath + "/" + shadersFile + "/" + asset + ".frag"))
    {
        
    }
    shaders[asset] = shader;
    return shaders[asset];
}
