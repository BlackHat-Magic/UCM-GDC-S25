// src/utils/tmx_parser.cpp
#include "tmx_parser.h"
#include "tilemap.h" // Include Tilemap header
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem> // Requires C++17

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

    // Store the base directory for resolving relative paths later
    try {
        // Use std::filesystem for robust path handling
        std::filesystem::path tmxFullPath(tmxPath);
        if (!std::filesystem::exists(tmxFullPath)) {
             std::cerr << "TMX file does not exist: " << tmxPath << std::endl;
             return false;
        }
        baseDir = tmxFullPath.parent_path().string();
        // Ensure trailing slash for easier concatenation later
        if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\') {
            baseDir += std::filesystem::path::preferred_separator;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing TMX path: " << e.what() << std::endl;
        baseDir = ""; // Fallback or handle error
        return false;
    }


    std::cout << "Loading TMX file: " << tmxPath << std::endl;
    std::cout << "Base directory set to: " << baseDir << std::endl;

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
        // Pass the TMX directory (baseDir) to parseTileset
        if (!parseTileset(tilesetElement, baseDir)) {
            std::cerr << "Failed to parse tileset" << std::endl;
            // Allow continuing if one tileset fails? Or return false?
            return false;
        }
    }

    // Parse layers
    for (tinyxml2::XMLElement* layerElement = mapElement->FirstChildElement("layer");
         layerElement;
         layerElement = layerElement->NextSiblingElement("layer")) {
        if (!parseLayer(layerElement)) {
            std::cerr << "Failed to parse layer" << std::endl;
            // Allow continuing?
            return false;
        }
    }

    // --- Load spritesheets for each tileset ---
    std::cout << "--- Loading Spritesheets ---" << std::endl;
    for (auto& [firstGid, tileset] : tilesets) {
        if (!tileset.imagePath.empty()) {
            std::string fullImagePathStr;
            std::filesystem::path imageRelativePath(tileset.imagePath); // Path from TSX/TMX

            try {
                if (tileset.source.empty()) {
                    // Embedded tileset: image path is relative to the TMX file's directory
                    std::filesystem::path tmxDir(baseDir);
                    std::filesystem::path imageFullPath = tmxDir / imageRelativePath;
                    fullImagePathStr = imageFullPath.lexically_normal().string();
                    std::cout << "  Tileset (Embedded): '" << tileset.name << "'" << std::endl;
                    std::cout << "    Image relative path (to TMX): " << tileset.imagePath << std::endl;

                } else {
                    // External tileset: image path is relative to the TSX file's directory
                    std::filesystem::path tmxDir(baseDir);
                    // Assume TSX source is relative to TMX dir (e.g., "../tilesets/file.tsx")
                    // Or just filename if in same dir, or relative path like "tilesets/file.tsx"
                    // The TMX file uses just the filename, implying they are in a known relative location.
                    // Your structure suggests "../tilesets/" relative to the TMX map dir.
                    std::filesystem::path tsxRelativePath(tileset.source); // e.g., "redshifted.tsx"
                    std::filesystem::path tsxFullPath = tmxDir / ".." / "tilesets" / tsxRelativePath; // Path to the TSX file
                    std::filesystem::path tsxDir = tsxFullPath.parent_path(); // Directory containing the TSX

                    std::filesystem::path imageFullPath = tsxDir / imageRelativePath; // Combine TSX dir + image path from TSX
                    fullImagePathStr = imageFullPath.lexically_normal().string(); // Normalize the path
                    std::cout << "  Tileset (External): '" << tileset.name << "' (Source: " << tileset.source << ")" << std::endl;
                    std::cout << "    TSX Full Path: " << tsxFullPath.string() << std::endl;
                    std::cout << "    Image relative path (to TSX): " << tileset.imagePath << std::endl;
                }

                std::cout << "    Attempting to load image: " << fullImagePathStr << std::endl;

                // Check if file exists before attempting to load
                 if (!std::filesystem::exists(fullImagePathStr)) {
                     std::cerr << "    ERROR: Image file not found at path: " << fullImagePathStr << std::endl;
                     // Decide how to handle: skip this tileset, return error, use placeholder?
                     // return false; // Make it fatal for now
                     continue; // Skip this tileset
                 }


                auto spritesheet = std::make_shared<Spritesheet>(
                    renderer, fullImagePathStr.c_str(), tileset.tileWidth, tileset.tileHeight);

                // Determine a unique key for the spritesheet map
                std::string key;
                 if (!tileset.source.empty()) {
                     // Use the TSX filename stem as the key for external tilesets
                     key = std::filesystem::path(tileset.source).stem().string();
                 } else {
                     // Use the tileset name for embedded tilesets
                     key = tileset.name;
                 }
                 if (key.empty()) { // Fallback if name/source stem is empty
                     key = "tileset_" + std::to_string(firstGid);
                 }

                spritesheets[key] = spritesheet;
                tileset.spritesheetKey = key; // Store the key used for this tileset
                std::cout << "    Successfully loaded spritesheet with key: '" << key << "'" << std::endl;

            } catch (const std::exception& e) {
                std::cerr << "    ERROR: Failed to load spritesheet: " << fullImagePathStr << std::endl;
                std::cerr << "      SDL Error: " << e.what() << std::endl;
                // Decide if this is fatal
                return false;
            }
        } else {
            std::cerr << "  Warning: Tileset '" << tileset.name << "' (First GID: " << firstGid << ") has no image path defined." << std::endl;
        }
    }
     std::cout << "--- Finished Loading Spritesheets ---" << std::endl;

    return true;
}

// Modified to accept tmxDir
bool TmxParser::parseTileset(tinyxml2::XMLElement* tilesetElement, const std::string& tmxDir) {
    int firstGid = tilesetElement->IntAttribute("firstgid", 0);
    if (firstGid <= 0) {
        std::cerr << "Invalid firstgid in tileset" << std::endl;
        return false;
    }

    // Check if this is an external tileset (has a source attribute)
    const char* source = tilesetElement->Attribute("source");
    if (source) {
        // External tileset: source path is relative to the TMX file
        std::filesystem::path tsxRelativePath(source);
        // Construct the full path assuming TSX is in "../tilesets/" relative to TMX dir
        std::filesystem::path tsxFullPath = std::filesystem::path(tmxDir) / ".." / "tilesets" / tsxRelativePath;
        std::string tsxFullPathStr = tsxFullPath.lexically_normal().string();

        std::string name = tsxRelativePath.stem().string(); // Use filename stem as name

        std::cout << "  Parsing External Tileset: " << source << std::endl;
        std::cout << "    Resolved TSX path: " << tsxFullPathStr << std::endl;

        TmxTileset tileset(name, firstGid, 0, 0); // Dimensions will be read from TSX
        tileset.source = source; // Store the original relative source path

        if (!std::filesystem::exists(tsxFullPathStr)) {
             std::cerr << "    ERROR: TSX file not found at: " << tsxFullPathStr << std::endl;
             return false;
        }

        if (!parseTsxFile(tileset, tsxFullPathStr)) {
            std::cerr << "    Failed to parse TSX file: " << tsxFullPathStr << std::endl;
            return false;
        }

        tilesets[firstGid] = tileset;
    } else {
        // Embedded tileset
        const char* name = tilesetElement->Attribute("name");
        int tileWidth = tilesetElement->IntAttribute("tilewidth", 0);
        int tileHeight = tilesetElement->IntAttribute("tileheight", 0);
        int tileCount = tilesetElement->IntAttribute("tilecount", 0);
        int columns = tilesetElement->IntAttribute("columns", 0);

        if (!name || tileWidth <= 0 || tileHeight <= 0) {
            std::cerr << "Invalid embedded tileset attributes" << std::endl;
            return false;
        }

        std::cout << "  Parsing Embedded Tileset: " << name << std::endl;

        TmxTileset tileset(name, firstGid, tileWidth, tileHeight);
        tileset.tileCount = tileCount;
        tileset.columns = columns;

        // Get the image element
        tinyxml2::XMLElement* imageElement = tilesetElement->FirstChildElement("image");
        if (imageElement) {
            const char* imagePath = imageElement->Attribute("source");
            if (imagePath) {
                tileset.imagePath = imagePath; // Store image path relative to TMX
                std::cout << "    Image path (relative to TMX): " << imagePath << std::endl;
            } else {
                 std::cerr << "    Warning: Embedded tileset '" << name << "' image element has no source attribute." << std::endl;
            }
        } else {
             std::cerr << "    Warning: Embedded tileset '" << name << "' has no image element." << std::endl;
        }

        tilesets[firstGid] = tileset;
    }

    return true;
}

bool TmxParser::parseTsxFile(TmxTileset& tileset, const std::string& tsxPathStr) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(tsxPathStr.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "    Failed to load TSX file: " << tsxPathStr << std::endl;
        std::cerr << "    Error: " << doc.ErrorStr() << std::endl;
        return false;
    }

    tinyxml2::XMLElement* tilesetElement = doc.FirstChildElement("tileset");
    if (!tilesetElement) {
        std::cerr << "    No tileset element found in TSX file: " << tsxPathStr << std::endl;
        return false;
    }

    // Parse tileset attributes (overwrite if already set, e.g., name)
    tileset.tileWidth = tilesetElement->IntAttribute("tilewidth", tileset.tileWidth);
    tileset.tileHeight = tilesetElement->IntAttribute("tileheight", tileset.tileHeight);
    tileset.tileCount = tilesetElement->IntAttribute("tilecount", tileset.tileCount);
    tileset.columns = tilesetElement->IntAttribute("columns", tileset.columns);
    // Optionally read name from TSX if not already set from source filename
    if (tileset.name.empty()) {
        const char* tsxName = tilesetElement->Attribute("name");
        if (tsxName) tileset.name = tsxName;
    }


    if (tileset.tileWidth <= 0 || tileset.tileHeight <= 0) {
        std::cerr << "    Invalid tileset dimensions in TSX file: " << tsxPathStr << std::endl;
        return false;
    }

    std::cout << "    TSX tileset dimensions: " << tileset.tileWidth << "x" << tileset.tileHeight << " pixels" << std::endl;
    std::cout << "    TSX tileset count: " << tileset.tileCount << " tiles, " << tileset.columns << " columns" << std::endl;

    // Get the image element
    tinyxml2::XMLElement* imageElement = tilesetElement->FirstChildElement("image");
    if (imageElement) {
        const char* imagePath = imageElement->Attribute("source");
        if (imagePath) {
            tileset.imagePath = imagePath; // Store image path relative to TSX
            std::cout << "    Image path (relative to TSX): " << imagePath << std::endl;
        } else {
             std::cerr << "    Warning: TSX tileset '" << tileset.name << "' image element has no source attribute." << std::endl;
        }
    } else {
         std::cerr << "    Warning: TSX tileset '" << tileset.name << "' has no image element." << std::endl;
    }

    // TODO: Parse per-tile properties (like collision) here if needed
    // for (tinyxml2::XMLElement* tileElement = tilesetElement->FirstChildElement("tile"); ...)

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
        std::cerr << "No data element found in layer: " << name << std::endl;
        return false; // Layer without data is invalid
    }

    // Check the encoding
    const char* encoding = dataElement->Attribute("encoding");
    if (!encoding || strcmp(encoding, "csv") != 0) {
        std::cerr << "Unsupported layer data encoding: " << (encoding ? encoding : "none") << " in layer " << name << std::endl;
        std::cerr << "Only CSV encoding is supported" << std::endl;
        return false; // Unsupported encoding
    }

    // Parse the CSV data
    const char* csvData = dataElement->GetText();
    if (!csvData) {
        std::cerr << "No CSV data found in layer: " << name << std::endl;
        // Treat as empty layer? Or error? Let's treat as empty for now.
        layer.tileData.assign(width * height, 0); // Fill with empty tiles (GID 0)
    } else {
        if (!parseCSV(csvData, layer.tileData)) {
            std::cerr << "Failed to parse CSV data for layer: " << name << std::endl;
            return false;
        }
        // Verify data size
        if (layer.tileData.size() != static_cast<size_t>(width * height)) {
             std::cerr << "Warning: CSV data size mismatch for layer '" << name << "'. Expected " << (width*height) << ", got " << layer.tileData.size() << std::endl;
             // Optionally resize or handle error
             layer.tileData.resize(width * height, 0); // Pad with empty tiles if short
        }
    }


    std::cout << "  Parsed " << layer.tileData.size() << " tile indices" << std::endl;

    // Add the layer to the list
    layers.push_back(layer);

    return true;
}

bool TmxParser::parseCSV(const std::string& csvData, std::vector<int>& tileData) {
    std::stringstream ss(csvData);
    std::string token;

    tileData.clear();
    tileData.reserve(mapWidth * mapHeight); // Pre-allocate roughly

    while (std::getline(ss, token, ',')) {
        // Trim whitespace (including newlines potentially embedded in the string)
        size_t first = token.find_first_not_of(" \n\r\t");
        if (first == std::string::npos) {
            // String is all whitespace, could happen at the end
            continue;
        }
        size_t last = token.find_last_not_of(" \n\r\t");
        token = token.substr(first, (last - first + 1));


        if (token.empty()) {
            // This case should ideally not happen after trimming if find_first_not_of worked
            continue;
        }

        try {
            // Parse the tile ID as an unsigned int to handle large values correctly
            // Tiled uses GID 0 for empty tiles.
            unsigned int rawGid = std::stoul(token);
            tileData.push_back(static_cast<int>(rawGid)); // Store the raw GID (potentially 0)
        } catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid argument: Failed to parse tile ID: '" << token << "'" << std::endl;
            return false;
        } catch (const std::out_of_range& oor) {
             std::cerr << "Out of range: Failed to parse tile ID: '" << token << "'" << std::endl;
             return false;
        }
    }

    return true;
}


CollisionLayer TmxParser::getCollisionLayerForName(const std::string& layerName) const {
    // Map layer names to collision layers based on your game's semantics
    // Make this case-insensitive for robustness
    std::string lowerLayerName = layerName;
    std::transform(lowerLayerName.begin(), lowerLayerName.end(), lowerLayerName.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (lowerLayerName == "abyss") {
        return CollisionLayer::LEVEL_PIT;
    } else if (lowerLayerName == "surfaces" || lowerLayerName == "ground") {
        // Decide if ground itself is collidable or just a base
        return CollisionLayer::LEVEL_FLOOR; // Or NONE if floor isn't solid
    } else if (lowerLayerName == "obstacles") {
        // Combine wall and obstacle types for simplicity?
        return CollisionLayer::LEVEL_WALL | CollisionLayer::LEVEL_OBSTACLE;
    } else if (lowerLayerName == "boundaries") {
        return CollisionLayer::LEVEL_BOUNDARY;
    } else if (lowerLayerName == "sides" || lowerLayerName.find("detail") != std::string::npos) {
        return CollisionLayer::NONE; // Purely aesthetic layers
    }

    // Default: Assume non-collidable if name doesn't match known patterns
    std::cout << "Warning: Unknown layer name for collision: '" << layerName << "', defaulting to NONE" << std::endl;
    return CollisionLayer::NONE;
}

// --- createTilemaps needs the flip logic re-integrated ---
std::vector<std::shared_ptr<Tilemap>> TmxParser::createTilemaps(SDL_Renderer* renderer) {
    std::vector<std::shared_ptr<Tilemap>> createdTilemaps;

    // Define the order of layers for rendering and collision accumulation
    // List all layers found in your TMX file in the desired draw order.
    std::vector<std::string> layerOrder = {
        "Abyss",
        "Sides",
        "Surfaces", // Use the name from your TMX
        "Detailing",
        "Detailing 2",
        "Obstacles",
        "Boundaries"
        // Add any other layers from your TMX here in draw order
    };

    std::cout << "--- Creating Tilemaps ---" << std::endl;
    for (const auto& name : layerOrder) {
        std::cout << "  Layer in order: " << name << std::endl;
    }

    // Create a map of collision layers for each LOCAL tile index.
    std::map<int, CollisionLayer> masterCollisionMap;

    // Process layers in the defined order
    for (const auto& layerName : layerOrder) {
        // Find the layer with this name
        auto layerIt = std::find_if(layers.begin(), layers.end(),
            [&layerName](const TmxLayer& layer) { return layer.name == layerName; });

        if (layerIt == layers.end()) {
            // Try finding case-insensitively or with common variations if needed
            bool foundAlt = false;
            for(auto it = layers.begin(); it != layers.end(); ++it) {
                std::string currentLower = it->name;
                std::transform(currentLower.begin(), currentLower.end(), currentLower.begin(), ::tolower);
                std::string targetLower = layerName;
                std::transform(targetLower.begin(), targetLower.end(), targetLower.begin(), ::tolower);
                if (currentLower == targetLower) {
                    layerIt = it;
                    foundAlt = true;
                    std::cout << "  (Note: Found layer '" << layerName << "' as '" << it->name << "')" << std::endl;
                    break;
                }
            }
            if (!foundAlt) {
                 std::cout << "  Layer '" << layerName << "' not found in parsed data, skipping." << std::endl;
                 continue;
            }
        }


        const TmxLayer& layer = *layerIt;
        if (!layer.visible) {
            std::cout << "  Skipping invisible layer: " << layer.name << std::endl;
            continue;
        }

        std::cout << "Processing layer: " << layer.name << std::endl;

        // Get the collision layer type assigned to this entire layer
        CollisionLayer layerCollisionType = getCollisionLayerForName(layer.name);
        std::cout << "  Assigned Collision Type: " << static_cast<int>(layerCollisionType) << std::endl;

        // Find the appropriate spritesheet and firstGid for this layer's tiles
        // This assumes tiles in one layer come from ONE tileset primarily.
        std::shared_ptr<Spritesheet> layerSpritesheet = nullptr;
        int layerFirstGid = 0;
        std::string associatedTilesetKey = ""; // The key used in the spritesheets map

        // Determine which tileset and spritesheet this layer uses
        // Find the first non-empty tile GID to determine the tileset
        int representativeGid = 0;
        for (int rawGid : layer.tileData) {
            if (rawGid > 0) {
                representativeGid = rawGid;
                break;
            }
        }

        if (representativeGid > 0) {
            unsigned actualGid = representativeGid & Tilemap::GID_MASK; // GID without flags

            // Find the tileset this GID belongs to
            const TmxTileset* foundTileset = nullptr;
            for (const auto& [fgid, ts] : tilesets) {
                // Check if the actual GID is within the range of this tileset
                // A GID belongs to the tileset with the largest firstgid <= actualGid
                if (actualGid >= static_cast<unsigned>(fgid)) {
                     if (foundTileset == nullptr || fgid > foundTileset->firstGid) {
                         foundTileset = &ts;
                     }
                }
            }

            if (foundTileset) {
                layerFirstGid = foundTileset->firstGid;
                associatedTilesetKey = foundTileset->spritesheetKey; // Use the stored key
                if (!associatedTilesetKey.empty() && spritesheets.count(associatedTilesetKey)) {
                    layerSpritesheet = spritesheets[associatedTilesetKey];
                    std::cout << "  Using spritesheet key: '" << associatedTilesetKey << "' (First GID: " << layerFirstGid << ")" << std::endl;
                } else {
                    std::cerr << "  Warning: No spritesheet found for key: '" << associatedTilesetKey << "' associated with tileset '" << foundTileset->name << "' needed by layer '" << layer.name << "'" << std::endl;
                }
            } else {
                 std::cerr << "  Warning: No tileset found for representative GID " << actualGid << " in layer '" << layer.name << "'" << std::endl;
            }
        } else {
             std::cout << "  Layer '" << layer.name << "' contains no tiles (or only GID 0)." << std::endl;
        }


        // If we found a spritesheet for this layer, create a tilemap
        if (layerSpritesheet) {
            // Populate the master collision map based on tiles in this layer
            if (layerCollisionType != CollisionLayer::NONE) {
                std::cout << "  Populating collision map for this layer..." << std::endl;
                int collisionTilesAdded = 0;
                for (int rawGid : layer.tileData) {
                     if (rawGid > 0) {
                         unsigned actualGid = rawGid & Tilemap::GID_MASK;
                         int localTileId = static_cast<int>(actualGid) - layerFirstGid;
                         if (localTileId >= 0) {
                             // Add or update the collision layer for this local tile ID
                             masterCollisionMap[localTileId] = layerCollisionType;
                             collisionTilesAdded++;
                         }
                     }
                }
                 std::cout << "    Added/updated " << collisionTilesAdded << " tiles in master collision map." << std::endl;
            }

            // Create the Tilemap instance for this layer
            // Pass the *master* collision map, which accumulates info from all layers processed so far.
            auto tilemap = std::make_shared<Tilemap>(
                layerSpritesheet.get(), tileWidth, tileHeight,
                layer.width, layer.height, masterCollisionMap); // Pass master map

            // Set the tiles in the tilemap, storing LOCAL ID | FLAGS
            for (int y = 0; y < layer.height; ++y) {
                for (int x = 0; x < layer.width; ++x) {
                    int index = y * layer.width + x;
                    if (index < layer.tileData.size()) {
                        int rawGid = layer.tileData[index];
                        if (rawGid > 0) {
                            // Extract flags
                            unsigned flipFlags = rawGid & ~Tilemap::GID_MASK;
                            // Extract actual GID
                            unsigned actualGid = rawGid & Tilemap::GID_MASK;

                            // Calculate local ID relative to the tileset used by this layer
                            int localTileId = static_cast<int>(actualGid) - layerFirstGid;

                            if (localTileId >= 0) {
                                // Combine local ID and original flags
                                int valueToStore = localTileId | flipFlags;
                                tilemap->setTile(x, y, valueToStore);
                            } else {
                                 std::cerr << "Warning: Calculated negative localTileId (" << localTileId << ") for rawGid " << rawGid << " and firstGid " << layerFirstGid << " in layer " << layer.name << std::endl;
                                 tilemap->setTile(x, y, 0); // Set as empty on error
                            }
                        } else {
                             tilemap->setTile(x, y, 0); // Empty tile (GID 0)
                        }
                    }
                }
            }

            // Set the opacity for this tilemap
            tilemap->setOpacity(layer.opacity);

            // Add the tilemap to the list
            createdTilemaps.push_back(tilemap);
            std::cout << "  Created tilemap for layer: " << layer.name << " (opacity: " << layer.opacity << ")" << std::endl;
        } else {
            std::cerr << "  Warning: Could not create tilemap for layer: '" << layer.name << "' (no valid spritesheet found/determined)." << std::endl;
        }
    }

    std::cout << "--- Finished Creating Tilemaps (" << createdTilemaps.size() << " created) ---" << std::endl;

    return createdTilemaps;
}
