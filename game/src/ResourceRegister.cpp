#include "raylib.h"
#include "rres.h"
#include "rres-raylib.h"

Sound RegisterSound(rresCentralDir centralDir, const char* resourceFile, const char* soundName) {
    int soundId = rresGetResourceId(centralDir, soundName);
    rresResourceChunk resourceChunk = rresLoadResourceChunk(resourceFile, soundId);
    Wave wave = LoadWaveFromResource(resourceChunk);
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    rresUnloadResourceChunk(resourceChunk);
    return sound;
}

Image RegisterImage(rresCentralDir centralDir, const char* resourceFile, const char* imageName) {
    int imageId = rresGetResourceId(centralDir, imageName);
    rresResourceChunk resourceChunk = rresLoadResourceChunk(resourceFile, imageId);
    Image image = LoadImageFromResource(resourceChunk);
    return image;
}