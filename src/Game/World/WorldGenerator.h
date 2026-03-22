// =============================================================================
// FILE: src/Game/World/WorldGenerator.h
// PURPOSE: Procedural world generation for Apocalypse realm
// =============================================================================

#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Math.h"
#include <vector>
#include <array>
#include <functional>

namespace Duality {

// ============================================================================
// Biome Types
// ============================================================================

enum class BiomeType : u8 {
    DERELICT_CITY,      // Collapsed megastructures
    INDUSTRIAL_WASTE,   // Factory complexes, toxic rivers
    CRATER_FIELD,       // Orbital bombardment zones
    BIOHAZARD_ZONE,     // Active weapon contamination
    ALIEN_TERRAFORMING, // Korath conversion zones
    WAR_MACHINE_GRAVEYARD, // Derelict mechs
    UNDERGROUND_BUNKER, // Safe zones (relatively)
    TOXIC_WASTELAND,    // General wasteland
    RADIATION_ZONE,     // High radiation areas
    ALIEN_STRUCTURE     // Alien ruins and installations
};

struct BiomeProperties {
    BiomeType type;
    Color primaryColor;
    Color secondaryColor;
    f32 heightScale;
    f32 vegetationDensity;
    f32 structureDensity;
    f32 dangerLevel;
    f32 radiationLevel;
    f32 resourceScarcity;
    std::string name;
};

// ============================================================================
// Terrain Data
// ============================================================================

struct TerrainVertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
    Vec4 color;
    u32 biomeId;
};

struct TerrainChunk {
    u32 x;
    u32 z;
    u32 size;
    std::vector<f32> heightmap;
    std::vector<BiomeType> biomeMap;
    std::vector<TerrainVertex> vertices;
    std::vector<u32> indices;
    bool needsUpdate = true;
    f32 minHeight = 0.0f;
    f32 maxHeight = 0.0f;
};

// ============================================================================
// World Generator
// ============================================================================

class WorldGenerator {
public:
    WorldGenerator();
    ~WorldGenerator();
    
    void Initialize(u64 seed, f32 worldSize = 100000.0f);
    void Shutdown();
    
    // Terrain generation
    f32 GetHeight(f32 x, f32 z) const;
    BiomeType GetBiome(f32 x, f32 z) const;
    f32 GetTemperature(f32 x, f32 z) const;
    f32 GetHumidity(f32 x, f32 z) const;
    f32 GetRadiation(f32 x, f32 z) const;
    
    // Chunk management
    TerrainChunk* GenerateChunk(u32 chunkX, u32 chunkZ, u32 resolution = 64);
    void UpdateChunk(TerrainChunk* chunk);
    void InvalidateChunk(u32 x, u32 z);
    
    // Structure generation
    void GenerateStructures(TerrainChunk* chunk);
    void GenerateRuins(TerrainChunk* chunk);
    void GenerateAlienStructures(TerrainChunk* chunk);
    void GenerateWrecks(TerrainChunk* chunk);
    
    // World streaming
    void UpdateStreaming(const Vec3& playerPosition, f32 viewDistance = 1000.0f);
    TerrainChunk* GetChunk(u32 x, u32 z);
    
    // World properties
    [[nodiscard]] u64 GetSeed() const { return m_seed; }
    [[nodiscard]] f32 GetWorldSize() const { return m_worldSize; }
    [[nodiscard]] u32 GetChunkCount() const { return static_cast<u32>(m_chunks.size()); }
    
private:
    // Noise functions
    f32 GenerateHeightNoise(f32 x, f32 z) const;
    f32 GenerateBiomeNoise(f32 x, f32 z) const;
    f32 GenerateDetailNoise(f32 x, f32 z) const;
    
    // Biome mapping
    BiomeType DetermineBiome(f32 x, f32 z, f32 height, f32 temperature, f32 humidity) const;
    void GenerateBiomeMap(TerrainChunk* chunk);
    
    // Mesh generation
    void GenerateMesh(TerrainChunk* chunk);
    void ComputeNormals(TerrainChunk* chunk);
    
    // Utilities
    u32 GetChunkKey(u32 x, u32 z) const { return (x << 16) | z; }
    
    u64 m_seed = 0;
    f32 m_worldSize = 100000.0f;
    f32 m_chunkSize = 256.0f;
    
    std::unordered_map<u32, TerrainChunk> m_chunks;
    std::vector<BiomeProperties> m_biomeProperties;
    
    // Noise generators
    class NoiseGenerator {
    public:
        void Initialize(u64 seed);
        f32 Simplex2D(f32 x, f32 y) const;
        f32 Simplex3D(f32 x, f32 y, f32 z) const;
        f32 FBM2D(f32 x, f32 y, i32 octaves, f32 lacunarity, f32 gain) const;
        f32 FBM3D(f32 x, f32 y, f32 z, i32 octaves, f32 lacunarity, f32 gain) const;
        
    private:
        static constexpr i32 PERM_SIZE = 512;
        std::array<i32, PERM_SIZE> m_perm;
    };
    
    NoiseGenerator m_heightNoise;
    NoiseGenerator m_biomeNoise;
    NoiseGenerator m_detailNoise;
    NoiseGenerator m_radiationNoise;
    
    Random m_random;
};

} // namespace Duality