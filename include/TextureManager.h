#pragma once
#include <map>
#include <string>
#include <raylib.h>

class TextureManager {
public:
    static Texture2D LoadTexture(const std::string& path);
    static void UnLoadTexture(const std::string& path);
    static Texture2D GetTexture(const std::string& path);
    static void Shutdown(); // para liberar todo al final

private:
    static std::map<std::string, Texture2D> textures;
};
