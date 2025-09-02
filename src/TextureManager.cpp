#include "TextureManager.h"

std::map<std::string, Texture2D> TextureManager::textures;

Texture2D TextureManager::LoadTexture(const std::string& path) {
    if (textures.find(path) == textures.end()) {
        textures[path] = ::LoadTexture(path.c_str());
    }
    return textures[path];
}

void TextureManager::UnLoadTexture(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) { 
        ::UnloadTexture(it->second);
        textures.erase(it);
    }
}

Texture2D TextureManager::GetTexture(const std::string& path) {
    return textures[path]; 
}

void TextureManager::Shutdown() {
    for (auto &kv : textures) {
        ::UnloadTexture(kv.second);
    }
    textures.clear();
}
