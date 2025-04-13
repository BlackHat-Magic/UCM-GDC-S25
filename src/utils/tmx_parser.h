// src/utils/tmx_parser.h
#pragma once
#include <SDL2/SDL.h>
#include "tinyxml2.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "spritesheet.h"
#include "collisions_defs.h"

// Forward declarations
class Tilemap;

// Represents a single layer from a TMX file
struct TmxLayer {
    std::string name;
    int width;
    int height;
    std::vector<int> tileData;
    bool visible;
    float opacity;
    
    // Constructor
    TmxLayer(const std::string& name, int width, int height, bool visible = true, float opacity = 1.0f)
        : name(name), width(width), height(height), visible(visible), opacity(opacity) {}
};

// Represents a tileset referenced by a TMX file
struct TmxTileset {
    std::string name;
    std::string source;
    int firstGid;
    int tileWidth;
    int tileHeight;
    int tileCount;
    int columns;
    std::string imagePath;
    
    // Default constructor (needed for std::map)
    TmxTileset()
        : name(""), source(""), firstGid(0), tileWidth(0), tileHeight(0),
          tileCount(0), columns(0), imagePath("") {}
    
    // Constructor
    TmxTileset(const std::string& name, int firstGid, int tileWidth, int tileHeight)
        : name(name), firstGid(firstGid), tileWidth(tileWidth), tileHeight(tileHeight),
          tileCount(0), columns(0), source(""), imagePath("") {}
};

// Main TMX parser class
class TmxParser {
public:
    TmxParser();
    ~TmxParser();
    
    // Load a TMX file and parse its contents
    bool loadTmx(const std::string& tmxPath, SDL_Renderer* renderer);
    
    // Create tilemaps from the parsed TMX data
    std::vector<std::shared_ptr<Tilemap>> createTilemaps(SDL_Renderer* renderer);
    
    // Get map dimensions
    int getMapWidth() const { return mapWidth; }
    int getMapHeight() const { return mapHeight; }
    int getTileWidth() const { return tileWidth; }
    int getTileHeight() const { return tileHeight; }
    
private:
    // Parse a layer from the TMX file
    bool parseLayer(tinyxml2::XMLElement* layerElement);
    
    // Parse a tileset from the TMX file
    bool parseTileset(tinyxml2::XMLElement* tilesetElement);
    
    // Parse a TSX file referenced by a tileset
    bool parseTsxFile(TmxTileset& tileset, const std::string& tsxPath);
    
    // Parse CSV data from a layer
    bool parseCSV(const std::string& csvData, std::vector<int>& tileData);
    
    // Map layer names to collision layers
    CollisionLayer getCollisionLayerForName(const std::string& layerName) const;
    
    // Map of loaded tilesets by firstGid
    std::map<int, TmxTileset> tilesets;
    
    // Vector of parsed layers
    std::vector<TmxLayer> layers;
    
    // Map dimensions
    int mapWidth;
    int mapHeight;
    int tileWidth;
    int tileHeight;
    
    // Base directory for resolving relative paths
    std::string baseDir;
    
    // Map of loaded spritesheets by tileset source
    std::map<std::string, std::shared_ptr<Spritesheet>> spritesheets;
};
