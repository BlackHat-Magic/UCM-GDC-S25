// src/utils/tmx_parser.cpp
#include "tmx_parser.h"
#include "tilemap.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>

TmxParser::TmxParser() 
    : mapWidth(0), mapHeight(0), tileWidth(0), tileHeight(0) {
}

TmxParser::~TmxParser() {
    // Spritesheets are managed by shared_ptr, no manual cleanup needed
}

bool TmxParser::loadTmx(const std::string& tmxPath, SDL_Renderer* renderer) {
    // Clear any previous data
    tilesets.clear();
    layers.clear();
    spritesheets.clear();
    
    // Store the base directory for resolving relative paths
    baseDir = std::filesystem::path(tmxPath).parent_path().string();
    if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\') {
        baseDir += '/';
    }
    
    std::cout << "Loading TMX file: " << tmxPath << std::endl;
    std::cout << "Base directory: " << baseDir << std::endl;
    
    // Load and parse the TMX file
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(tmxPath.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load TMX file: " << tmxPath << std::endl;
        std::cerr << "Error: " << doc.ErrorStr() << std::endl;
        return false;
    }
    
    // Get the map element
    tinyxml2::XMLElement* mapElement = doc.FirstChildElement("map");
    if (!mapElement) {
        std::cerr << "No map element found in TMX file" << std::endl;
        return false;
    }
    
    // Parse map attributes
    mapWidth = mapElement->IntAttribute("width", 0);
    mapHeight = mapElement->IntAttribute("height", 0);
    tileWidth = mapElement->IntAttribute("tilewidth", 0);
    tileHeight = mapElement->IntAttribute("tileheight", 0);
    
    if (mapWidth <= 0 || mapHeight <= 0 || tileWidth <= 0 || tileHeight <= 0) {
        std::cerr << "Invalid map dimensions in TMX file" << std::endl;
        return false;
    }
    
    std::cout << "Map dimensions: " << mapWidth << "x" << mapHeight << " tiles" << std::endl;
    std::cout << "Tile dimensions: " << tileWidth << "x" << tileHeight << " pixels" << std::endl;
    
    // Parse tilesets
    for (tinyxml2::XMLElement* tilesetElement = mapElement->FirstChildElement("tileset");
         tilesetElement;
         tilesetElement = tilesetElement->NextSiblingElement("tileset")) {
        if (!parseTileset(tilesetElement)) {
            std::cerr << "Failed to parse tileset" << std::endl;
            return false;
        }
    }
    
    // Parse layers
    for (tinyxml2::XMLElement* layerElement = mapElement->FirstChildElement("layer");
         layerElement;
         layerElement = layerElement->NextSiblingElement("layer")) {
        if (!parseLayer(layerElement)) {
            std::cerr << "Failed to parse layer" << std::endl;
            return false;
        }
    }
    
    // Load spritesheets for each tileset
    for (auto& [firstGid, tileset] : tilesets) {
        if (!tileset.imagePath.empty()) {
            // For embedded tilesets, the image path is relative to the TMX file
            std::string fullImagePath;
            if (tileset.source.empty()) {
                // Embedded tileset
                fullImagePath = baseDir + "../" + tileset.imagePath;
            } else {
                // External tileset - image path is relative to the TSX file
                std::string tsxDir = std::filesystem::path(baseDir + "../tilesets/").string();
                fullImagePath = tsxDir + tileset.imagePath;
            }
            
            std::cout << "Loading spritesheet: " << fullImagePath << std::endl;
            
            try {
                auto spritesheet = std::make_shared<Spritesheet>(
                    renderer, fullImagePath.c_str(), tileset.tileWidth, tileset.tileHeight);
                spritesheets[tileset.source.empty() ? tileset.name : tileset.source] = spritesheet;
                std::cout << "Successfully loaded spritesheet" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Failed to load spritesheet: " << fullImagePath << std::endl;
                std::cerr << "Error: " << e.what() << std::endl;
                return false;
            }
        } else {
            std::cerr << "Warning: Tileset has no image path: " << tileset.name << std::endl;
        }
    }
    
    return true;
}

bool TmxParser::parseTileset(tinyxml2::XMLElement* tilesetElement) {
    int firstGid = tilesetElement->IntAttribute("firstgid", 0);
    if (firstGid <= 0) {
        std::cerr << "Invalid firstgid in tileset" << std::endl;
        return false;
    }
    
    // Check if this is an external tileset (has a source attribute)
    const char* source = tilesetElement->Attribute("source");
    if (source) {
        // This is an external tileset, parse the TSX file
        std::string tsxPath = baseDir + "../tilesets/" + source;
        std::string name = std::filesystem::path(source).stem().string();
        
        std::cout << "External tileset found: " << source << std::endl;
        std::cout << "TSX path: " << tsxPath << std::endl;
        
        TmxTileset tileset(name, firstGid, 0, 0);
        tileset.source = source;
        
        if (!parseTsxFile(tileset, tsxPath)) {
            std::cerr << "Failed to parse TSX file: " << tsxPath << std::endl;
            return false;
        }
        
        tilesets[firstGid] = tileset;
    } else {
        // This is an embedded tileset
        const char* name = tilesetElement->Attribute("name");
        int tileWidth = tilesetElement->IntAttribute("tilewidth", 0);
        int tileHeight = tilesetElement->IntAttribute("tileheight", 0);
        int tileCount = tilesetElement->IntAttribute("tilecount", 0);
        int columns = tilesetElement->IntAttribute("columns", 0);
        
        if (!name || tileWidth <= 0 || tileHeight <= 0) {
            std::cerr << "Invalid tileset attributes" << std::endl;
            return false;
        }
        
        std::cout << "Embedded tileset found: " << name << std::endl;
        
        TmxTileset tileset(name, firstGid, tileWidth, tileHeight);
        tileset.tileCount = tileCount;
        tileset.columns = columns;
        
        // Get the image element
        tinyxml2::XMLElement* imageElement = tilesetElement->FirstChildElement("image");
        if (imageElement) {
            const char* imagePath = imageElement->Attribute("source");
            if (imagePath) {
                tileset.imagePath = imagePath;
                std::cout << "Image path: " << imagePath << std::endl;
            }
        }
        
        tilesets[firstGid] = tileset;
    }
    
    return true;
}

bool TmxParser::parseTsxFile(TmxTileset& tileset, const std::string& tsxPath) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(tsxPath.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load TSX file: " << tsxPath << std::endl;
        std::cerr << "Error: " << doc.ErrorStr() << std::endl;
        return false;
    }
    
    tinyxml2::XMLElement* tilesetElement = doc.FirstChildElement("tileset");
    if (!tilesetElement) {
        std::cerr << "No tileset element found in TSX file" << std::endl;
        return false;
    }
    
    // Parse tileset attributes
    tileset.tileWidth = tilesetElement->IntAttribute("tilewidth", 0);
    tileset.tileHeight = tilesetElement->IntAttribute("tileheight", 0);
    tileset.tileCount = tilesetElement->IntAttribute("tilecount", 0);
    tileset.columns = tilesetElement->IntAttribute("columns", 0);
    
    if (tileset.tileWidth <= 0 || tileset.tileHeight <= 0) {
        std::cerr << "Invalid tileset dimensions in TSX file" << std::endl;
        return false;
    }
    
    std::cout << "TSX tileset dimensions: " << tileset.tileWidth << "x" << tileset.tileHeight << " pixels" << std::endl;
    std::cout << "TSX tileset count: " << tileset.tileCount << " tiles, " << tileset.columns << " columns" << std::endl;
    
    // Get the image element
    tinyxml2::XMLElement* imageElement = tilesetElement->FirstChildElement("image");
    if (imageElement) {
        const char* imagePath = imageElement->Attribute("source");
        if (imagePath) {
            tileset.imagePath = imagePath;
            std::cout << "TSX image path: " << imagePath << std::endl;
        }
    }
    
    return true;
}

bool TmxParser::parseLayer(tinyxml2::XMLElement* layerElement) {
    const char* name = layerElement->Attribute("name");
    int width = layerElement->IntAttribute("width", 0);
    int height = layerElement->IntAttribute("height", 0);
    float opacity = layerElement->FloatAttribute("opacity", 1.0f);
    int visibleAttr = layerElement->IntAttribute("visible", 1);
    bool visible = (visibleAttr != 0);
    
    if (!name || width <= 0 || height <= 0) {
        std::cerr << "Invalid layer attributes" << std::endl;
        return false;
    }
    
    std::cout << "Parsing layer: " << name << " (" << width << "x" << height << ")" << std::endl;
    std::cout << "  Opacity: " << opacity << ", Visible: " << (visible ? "yes" : "no") << std::endl;
    
    TmxLayer layer(name, width, height, visible, opacity);
    
    // Get the data element
    tinyxml2::XMLElement* dataElement = layerElement->FirstChildElement("data");
    if (!dataElement) {
        std::cerr << "No data element found in layer" << std::endl;
        return false;
    }
    
    // Check the encoding
    const char* encoding = dataElement->Attribute("encoding");
    if (!encoding || strcmp(encoding, "csv") != 0) {
        std::cerr << "Unsupported layer data encoding: " << (encoding ? encoding : "none") << std::endl;
        std::cerr << "Only CSV encoding is supported" << std::endl;
        return false;
    }
    
    // Parse the CSV data
    const char* csvData = dataElement->GetText();
    if (!csvData) {
        std::cerr << "No CSV data found in layer" << std::endl;
        return false;
    }
    
    if (!parseCSV(csvData, layer.tileData)) {
        std::cerr << "Failed to parse CSV data" << std::endl;
        return false;
    }
    
    std::cout << "  Parsed " << layer.tileData.size() << " tile indices" << std::endl;
    
    // Add the layer to the list
    layers.push_back(layer);
    
    return true;
}

bool TmxParser::parseCSV(const std::string& csvData, std::vector<int>& tileData) {
    std::istringstream ss(csvData);
    std::string token;
    
    tileData.clear();
    
    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \n\r\t"));
        token.erase(token.find_last_not_of(" \n\r\t") + 1);
        
        if (token.empty()) {
            continue;
        }
        
        try {
            int tileId = std::stoi(token);
            tileData.push_back(tileId);
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse tile ID: " << token << std::endl;
            return false;
        }
    }
    
    return true;
}

CollisionLayer TmxParser::getCollisionLayerForName(const std::string& layerName) const {
    // Map layer names to collision layers based on your game's semantics
    if (layerName == "Abyss") {
        return CollisionLayer::LEVEL_PIT;
    } else if (layerName == "Surface" || layerName == "Ground") {
        return CollisionLayer::LEVEL_FLOOR;
    } else if (layerName == "Obstacles") {
        return CollisionLayer::LEVEL_OBSTACLE;
    } else if (layerName == "Boundaries") {
        return CollisionLayer::LEVEL_BOUNDARY;
    } else if (layerName == "Sides" || layerName == "Details1" || layerName == "Details-1" || 
               layerName == "Details2" || layerName == "Details-2" || 
               layerName == "Detailing" || layerName == "Detailing 2") {
        return CollisionLayer::NONE; // Purely aesthetic layers
    }
    
    // Default
    std::cout << "Warning: Unknown layer name: " << layerName << ", defaulting to NONE" << std::endl;
    return CollisionLayer::NONE;
}

std::vector<std::shared_ptr<Tilemap>> TmxParser::createTilemaps(SDL_Renderer* renderer) {
    std::vector<std::shared_ptr<Tilemap>> tilemaps;
    
    // Define the order of layers for rendering and collision
    std::vector<std::string> layerOrder = {
        "Abyss",
        "Sides",
        "Ground", "Surface",
        "Details1", "Details-1", "Detailing",
        "Details2", "Details-2", "Detailing 2",
        "Obstacles",
        "Boundaries"
    };
    
    std::cout << "Creating tilemaps in the following order:" << std::endl;
    for (const auto& name : layerOrder) {
        std::cout << "  " << name << std::endl;
    }
    
    // Create a map of collision layers for each tile index
    std::map<int, CollisionLayer> collisionMap;
    
    // Process layers in the defined order
    for (const auto& layerName : layerOrder) {
        // Find the layer with this name
        auto layerIt = std::find_if(layers.begin(), layers.end(),
            [&layerName](const TmxLayer& layer) { return layer.name == layerName; });
        
        if (layerIt == layers.end()) {
            // Layer not found, skip
            continue;
        }
        
        const TmxLayer& layer = *layerIt;
        if (!layer.visible) {
            // Skip invisible layers
            std::cout << "Skipping invisible layer: " << layer.name << std::endl;
            continue;
        }
        
        std::cout << "Processing layer: " << layer.name << std::endl;
        
        // Get the collision layer for this layer name
        CollisionLayer layerCollision = getCollisionLayerForName(layer.name);
        
        // Find the appropriate spritesheet for this layer
        std::shared_ptr<Spritesheet> layerSpritesheet;
        int layerFirstGid = 0;
        
        // For each tile in the layer, determine which tileset it belongs to
        for (int tileId : layer.tileData) {
            if (tileId <= 0) {
                // Empty tile, skip
                continue;
            }
            
            // Find the tileset that contains this tile
            int tilesetFirstGid = 0;
            for (const auto& [firstGid, tileset] : tilesets) {
                if (firstGid <= tileId && (tilesetFirstGid == 0 || firstGid > tilesetFirstGid)) {
                    tilesetFirstGid = firstGid;
                }
            }
            
            if (tilesetFirstGid > 0) {
                const TmxTileset& tileset = tilesets[tilesetFirstGid];
                
                // If we haven't selected a spritesheet for this layer yet, use this one
                if (!layerSpritesheet) {
                    std::string spritesheetKey = tileset.source.empty() ? tileset.name : tileset.source;
                    if (spritesheets.count(spritesheetKey) > 0) {
                        layerSpritesheet = spritesheets[spritesheetKey];
                        layerFirstGid = tilesetFirstGid;
                        std::cout << "  Using spritesheet: " << spritesheetKey << " (firstGid: " << layerFirstGid << ")" << std::endl;
                    } else {
                        std::cerr << "  Warning: No spritesheet found for key: " << spritesheetKey << std::endl;
                    }
                }
                
                // Add this tile to the collision map if it has a collision layer
                if (layerCollision != CollisionLayer::NONE) {
                    collisionMap[tileId] = layerCollision;
                }
            }
        }
        
        // If we found a spritesheet for this layer, create a tilemap
        if (layerSpritesheet) {
            auto tilemap = std::make_shared<Tilemap>(
                layerSpritesheet.get(), tileWidth, tileHeight, 
                layer.width, layer.height, collisionMap);
            
            // Set the tiles in the tilemap
            for (int y = 0; y < layer.height; ++y) {
                for (int x = 0; x < layer.width; ++x) {
                    int index = y * layer.width + x;
                    if (index < layer.tileData.size()) {
                        int tileId = layer.tileData[index];
                        if (tileId > 0) {
                            // Convert from global tile ID to local tile ID for the spritesheet
                            int localTileId = tileId - layerFirstGid;
                            tilemap->setTile(x, y, localTileId);
                        }
                    }
                }
            }
            
            // Set the opacity for this tilemap
            tilemap->setOpacity(layer.opacity);
            
            // Add the tilemap to the list
            tilemaps.push_back(tilemap);
            std::cout << "  Created tilemap for layer: " << layer.name << " (opacity: " << layer.opacity << ")" << std::endl;
        } else {
            std::cerr << "  Warning: No spritesheet found for layer: " << layer.name << std::endl;
        }
    }
    
    std::cout << "Created " << tilemaps.size() << " tilemaps" << std::endl;
    
    return tilemaps;
}
