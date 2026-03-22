// =============================================================================
// FILE: src/Game/World/WorldGenerator.cpp
// PURPOSE: Procedural world generation implementation
// =============================================================================

#include "WorldGenerator.h"
#include "Engine/Core/Logger.h"
#include <cmath>

namespace Duality {

// ============================================================================
// Noise Generator Implementation
// ============================================================================

void WorldGenerator::NoiseGenerator::Initialize(u64 seed) {
    // Initialize permutation table based on seed
    std::array<i32, 256> p;
    for (i32 i = 0; i < 256; i++) {
        p[i] = i;
    }
    
    // Fisher-Yates shuffle with seed
    Random rng(seed);
    for (i32 i = 255; i > 0; i--) {
        i32 j = rng.RangeInt(0, i);
        std::swap(p[i], p[j]);
    }
    
    // Duplicate for easier indexing
    for (i32 i = 0; i < 256; i++) {
        m_perm[i] = p[i];
        m_perm[i + 256] = p[i];
    }
}

f32 WorldGenerator::NoiseGenerator::Simplex2D(f32 x, f32 y) const {
    const f32 F2 = 0.5f * (std::sqrt(3.0f) - 1.0f);
    const f32 G2 = (3.0f - std::sqrt(3.0f)) / 6.0f;
    
    f32 n0 = 0.0f, n1 = 0.0f, n2 = 0.0f;
    
    f32 s = (x + y) * F2;
    i32 i = static_cast<i32>(std::floor(x + s));
    i32 j = static_cast<i32>(std::floor(y + s));
    f32 t = (i + j) * G2;
    f32 X0 = i - t;
    f32 Y0 = j - t;
    f32 x0 = x - X0;
    f32 y0 = y - Y0;
    
    i32 i1, j1;
    if (x0 > y0) { i1 = 1; j1 = 0; }
    else { i1 = 0; j1 = 1; }
    
    f32 x1 = x0 - i1 + G2;
    f32 y1 = y0 - j1 + G2;
    f32 x2 = x0 - 1.0f + 2.0f * G2;
    f32 y2 = y0 - 1.0f + 2.0f * G2;
    
    i32 ii = i & 255;
    i32 jj = j & 255;
    
    i32 gi0 = m_perm[ii + m_perm[jj]] % 12;
    i32 gi1 = m_perm[ii + i1 + m_perm[jj + j1]] % 12;
    i32 gi2 = m_perm[ii + 1 + m_perm[jj + 1]] % 12;
    
    static const f32 grad3[12][3] = {
        {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
        {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
        {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
    };
    
    auto dot = [&](i32 g, f32 x, f32 y) {
        return grad3[g][0] * x + grad3[g][1] * y;
    };
    
    f32 t0 = 0.5f - x0 * x0 - y0 * y0;
    if (t0 >= 0) {
        t0 *= t0;
        n0 = t0 * t0 * dot(gi0, x0, y0);
    }
    
    f32 t1 = 0.5f - x1 * x1 - y1 * y1;
    if (t1 >= 0) {
        t1 *= t1;
        n1 = t1 * t1 * dot(gi1, x1, y1);
    }
    
    f32 t2 = 0.5f - x2 * x2 - y2 * y2;
    if (t2 >= 0) {
        t2 *= t2;
        n2 = t2 * t2 * dot(gi2, x2, y2);
    }
    
    return 70.0f * (n0 + n1 + n2);
}

f32 WorldGenerator::NoiseGenerator::FBM2D(f32 x, f32 y, i32 octaves, f32 lacunarity, f32 gain) const {
    f32 value = 0.0f;
    f32 amplitude = 1.0f;
    f32 frequency = 1.0f;
    f32 maxValue = 0.0f;
    
    for (i32 i = 0; i < octaves; i++) {
        value += Simplex2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }
    
    return value / maxValue;
}

// ============================================================================
// World Generator Implementation
// ============================================================================

WorldGenerator::WorldGenerator() {
    // Initialize biome properties
    m_biomeProperties = {
        {BiomeType::DERELICT_CITY, Color(80, 70, 60), Color(100, 80, 70), 20.0f, 0.05f, 0.8f, 0.7f, 0.3f, 0.2f, "Derelict City"},
        {BiomeType::INDUSTRIAL_WASTE, Color(60, 55, 50), Color(80, 70, 60), 15.0f, 0.02f, 0.6f, 0.8f, 0.5f, 0.3f, "Industrial Waste"},
        {BiomeType::CRATER_FIELD, Color(70, 65, 55), Color(90, 80, 65), 10.0f, 0.0f, 0.2f, 0.6f, 0.4f, 0.4f, "Crater Field"},
        {BiomeType::BIOHAZARD_ZONE, Color(50, 70, 45), Color(70, 100, 60), 8.0f, 0.3f, 0.4f, 0.9f, 0.8f, 0.1f, "Biohazard Zone"},
        {BiomeType::ALIEN_TERRAFORMING, Color(100, 80, 120), Color(120, 90, 140), 25.0f, 0.1f, 0.5f, 0.9f, 0.6f, 0.15f, "Alien Terraforming"},
        {BiomeType::WAR_MACHINE_GRAVEYARD, Color(70, 65, 60), Color(90, 80, 70), 12.0f, 0.0f, 0.7f, 0.7f, 0.5f, 0.25f, "War Machine Graveyard"},
        {BiomeType::UNDERGROUND_BUNKER, Color(50, 55, 60), Color(70, 75, 80), 5.0f, 0.0f, 0.1f, 0.3f, 0.1f, 0.6f, "Underground Bunker"},
        {BiomeType::TOXIC_WASTELAND, Color(85, 75, 60), Color(110, 90, 70), 8.0f, 0.01f, 0.2f, 0.8f, 0.7f, 0.2f, "Toxic Wasteland"},
        {BiomeType::RADIATION_ZONE, Color(70, 80, 60), Color(90, 100, 75), 10.0f, 0.0f, 0.15f, 1.0f, 0.9f, 0.1f, "Radiation Zone"},
        {BiomeType::ALIEN_STRUCTURE, Color(120, 90, 150), Color(140, 110, 170), 30.0f, 0.0f, 0.9f, 0.8f, 0.4f, 0.3f, "Alien Structure"}
    };
}

WorldGenerator::~WorldGenerator() {
    Shutdown();
}

void WorldGenerator::Initialize(u64 seed, f32 worldSize) {
    m_seed = seed;
    m_worldSize = worldSize;
    m_random = Random(seed);
    
    LOG_INFO("Initializing world generator with seed: {}", seed);
    LOG_INFO("World size: {} km x {} km", worldSize / 1000.0f, worldSize / 1000.0f);
    
    // Initialize noise generators
    m_heightNoise.Initialize(seed);
    m_biomeNoise.Initialize(seed + 1);
    m_detailNoise.Initialize(seed + 2);
    m_radiationNoise.Initialize(seed + 3);
    
    LOG_INFO("World generator initialized");
}

void WorldGenerator::Shutdown() {
    m_chunks.clear();
    LOG_INFO("World generator shutdown");
}

f32 WorldGenerator::GetHeight(f32 x, f32 z) const {
    // Clamp to world bounds
    x = std::clamp(x, -m_worldSize * 0.5f, m_worldSize * 0.5f);
    z = std::clamp(z, -m_worldSize * 0.5f, m_worldSize * 0.5f);
    
    f32 nx = x / m_worldSize;
    f32 nz = z / m_worldSize;
    
    // Base terrain using FBM
    f32 baseHeight = m_heightNoise.FBM2D(nx * 4.0f, nz * 4.0f, 6, 2.0f, 0.5f);
    
    // Add mid-frequency details
    f32 midHeight = m_detailNoise.FBM2D(nx * 16.0f, nz * 16.0f, 4, 2.0f, 0.5f) * 0.3f;
    
    // Add high-frequency details
    f32 highHeight = m_detailNoise.Simplex2D(nx * 64.0f, nz * 64.0f) * 0.1f;
    
    // Combine heights
    f32 height = (baseHeight * 100.0f) + (midHeight * 30.0f) + (highHeight * 10.0f);
    
    // Apply biome-specific modifications
    BiomeType biome = GetBiome(x, z);
    const auto& props = m_biomeProperties[static_cast<u32>(biome)];
    height += props.heightScale;
    
    // Add crater effects
    for (i32 i = -3; i <= 3; i++) {
        for (i32 j = -3; j <= 3; j++) {
            f32 craterX = (static_cast<f32>(i) * 500.0f);
            f32 craterZ = (static_cast<f32>(j) * 500.0f);
            f32 dist = std::sqrt((x - craterX) * (x - craterX) + (z - craterZ) * (z - craterZ));
            if (dist < 200.0f) {
                height -= 20.0f * (1.0f - dist / 200.0f);
            }
        }
    }
    
    return std::max(0.0f, height);
}

BiomeType WorldGenerator::GetBiome(f32 x, f32 z) const {
    f32 nx = x / m_worldSize;
    f32 nz = z / m_worldSize;
    
    // Generate biome noise
    f32 biomeValue = m_biomeNoise.FBM2D(nx * 8.0f, nz * 8.0f, 4, 2.0f, 0.5f);
    
    // Add war influence (areas of high destruction)
    f32 warInfluence = m_biomeNoise.Simplex2D(nx * 2.0f, nz * 2.0f);
    warInfluence = (warInfluence + 1.0f) * 0.5f;
    
    // Determine biome based on noise and location
    f32 height = GetHeight(x, z);
    f32 temperature = GetTemperature(x, z);
    f32 humidity = GetHumidity(x, z);
    
    return DetermineBiome(x, z, height, temperature, humidity);
}

f32 WorldGenerator::GetTemperature(f32 x, f32 z) const {
    f32 nx = x / m_worldSize;
    f32 nz = z / m_worldSize;
    
    // Base temperature decreases with latitude
    f32 latitude = std::abs(nz - 0.5f) * 2.0f;
    f32 baseTemp = 1.0f - latitude * 0.8f;
    
    // Add noise variation
    f32 noiseVar = m_heightNoise.Simplex2D(nx * 2.0f, nz * 2.0f) * 0.2f;
    
    // War zones are hotter
    f32 warHeat = m_biomeNoise.Simplex2D(nx * 4.0f, nz * 4.0f);
    warHeat = std::max(0.0f, warHeat) * 0.3f;
    
    return std::clamp(baseTemp + noiseVar + warHeat, 0.0f, 1.0f);
}

f32 WorldGenerator::GetHumidity(f32 x, f32 z) const {
    f32 nx = x / m_worldSize;
    f32 nz = z / m_worldSize;
    
    // Humidity based on distance to water features
    f32 waterDist = std::abs(m_heightNoise.Simplex2D(nx * 0.5f, nz * 0.5f));
    
    // Add wind patterns
    f32 windEffect = std::sin(nx * Math::Constants::PI * 4.0f) * 0.3f;
    
    // Biohazard zones have higher humidity
    f32 bioHazard = m_biomeNoise.Simplex2D(nx * 3.0f, nz * 3.0f);
    bioHazard = std::max(0.0f, bioHazard) * 0.4f;
    
    return std::clamp(waterDist * 0.6f + windEffect + bioHazard, 0.0f, 1.0f);
}

f32 WorldGenerator::GetRadiation(f32 x, f32 z) const {
    f32 nx = x / m_worldSize;
    f32 nz = z / m_worldSize;
    
    // Base radiation from war
    f32 baseRad = m_radiationNoise.FBM2D(nx * 16.0f, nz * 16.0f, 3, 2.0f, 0.5f);
    
    // Crater zones have higher radiation
    f32 craterRad = 0.0f;
    for (i32 i = -2; i <= 2; i++) {
        for (i32 j = -2; j <= 2; j++) {
            f32 craterX = (static_cast<f32>(i) * 1000.0f);
            f32 craterZ = (static_cast<f32>(j) * 1000.0f);
            f32 dist = std::sqrt((x - craterX) * (x - craterX) + (z - craterZ) * (z - craterZ));
            if (dist < 300.0f) {
                craterRad += 0.5f * (1.0f - dist / 300.0f);
            }
        }
    }
    
    // Alien structures have radiation
    f32 alienRad = m_biomeNoise.Simplex2D(nx * 5.0f, nz * 5.0f);
    alienRad = std::max(0.0f, alienRad) * 0.4f;
    
    return std::clamp(baseRad * 0.7f + craterRad + alienRad, 0.0f, 1.0f);
}

BiomeType WorldGenerator::DetermineBiome(f32 x, f32 z, f32 height, f32 temperature, f32 humidity) const {
    f32 nx = x / m_worldSize;
    f32 nz = z / m_worldSize;
    
    // Check for special zones
    f32 alienInfluence = m_biomeNoise.Simplex2D(nx * 0.2f, nz * 0.2f);
    if (alienInfluence > 0.7f && height > 50.0f) {
        return BiomeType::ALIEN_STRUCTURE;
    }
    
    f32 warInfluence = m_biomeNoise.Simplex2D(nx * 0.5f, nz * 0.5f);
    
    // Determine biome based on parameters
    if (height > 80.0f) {
        if (warInfluence > 0.6f) return BiomeType::WAR_MACHINE_GRAVEYARD;
        if (alienInfluence > 0.5f) return BiomeType::ALIEN_TERRAFORMING;
        return BiomeType::DERELICT_CITY;
    }
    
    if (height > 50.0f) {
        if (warInfluence > 0.5f) return BiomeType::INDUSTRIAL_WASTE;
        return BiomeType::CRATER_FIELD;
    }
    
    if (humidity > 0.7f && warInfluence > 0.6f) {
        return BiomeType::BIOHAZARD_ZONE;
    }
    
    if (GetRadiation(x, z) > 0.7f) {
        return BiomeType::RADIATION_ZONE;
    }
    
    if (height < 20.0f) {
        return BiomeType::UNDERGROUND_BUNKER;
    }
    
    return BiomeType::TOXIC_WASTELAND;
}

TerrainChunk* WorldGenerator::GenerateChunk(u32 chunkX, u32 chunkZ, u32 resolution) {
    u32 key = GetChunkKey(chunkX, chunkZ);
    
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        return &it->second;
    }
    
    TerrainChunk chunk;
    chunk.x = chunkX;
    chunk.z = chunkZ;
    chunk.size = resolution;
    
    f32 worldX = chunkX * m_chunkSize;
    f32 worldZ = chunkZ * m_chunkSize;
    
    // Generate heightmap
    chunk.heightmap.resize(resolution * resolution);
    chunk.biomeMap.resize(resolution * resolution);
    
    f32 minH = Math::Constants::MAX_F32;
    f32 maxH = -Math::Constants::MAX_F32;
    
    for (u32 i = 0; i < resolution; i++) {
        for (u32 j = 0; j < resolution; j++) {
            f32 x = worldX + (static_cast<f32>(i) / static_cast<f32>(resolution - 1)) * m_chunkSize;
            f32 z = worldZ + (static_cast<f32>(j) / static_cast<f32>(resolution - 1)) * m_chunkSize;
            
            f32 height = GetHeight(x, z);
            BiomeType biome = GetBiome(x, z);
            
            chunk.heightmap[i * resolution + j] = height;
            chunk.biomeMap[i * resolution + j] = biome;
            
            minH = std::min(minH, height);
            maxH = std::max(maxH, height);
        }
    }
    
    chunk.minHeight = minH;
    chunk.maxHeight = maxH;
    
    // Generate mesh
    GenerateMesh(&chunk);
    GenerateStructures(&chunk);
    
    m_chunks[key] = std::move(chunk);
    
    return &m_chunks[key];
}

void WorldGenerator::GenerateMesh(TerrainChunk* chunk) {
    u32 resolution = chunk->size;
    f32 step = m_chunkSize / static_cast<f32>(resolution - 1);
    
    chunk->vertices.clear();
    chunk->indices.clear();
    
    // Generate vertices
    for (u32 i = 0; i < resolution; i++) {
        for (u32 j = 0; j < resolution; j++) {
            f32 x = chunk->x * m_chunkSize + i * step;
            f32 z = chunk->z * m_chunkSize + j * step;
            f32 y = chunk->heightmap[i * resolution + j];
            
            BiomeType biome = chunk->biomeMap[i * resolution + j];
            const auto& props = m_biomeProperties[static_cast<u32>(biome)];
            
            TerrainVertex vertex;
            vertex.position = Vec3(x, y, z);
            vertex.uv = Vec2(i / static_cast<f32>(resolution - 1), j / static_cast<f32>(resolution - 1));
            vertex.biomeId = static_cast<u32>(biome);
            
            // Color based on biome and height
            f32 t = (y - chunk->minHeight) / (chunk->maxHeight - chunk->minHeight);
            vertex.color = Vec4(
                Math::Lerp(props.primaryColor.r / 255.0f, props.secondaryColor.r / 255.0f, t),
                Math::Lerp(props.primaryColor.g / 255.0f, props.secondaryColor.g / 255.0f, t),
                Math::Lerp(props.primaryColor.b / 255.0f, props.secondaryColor.b / 255.0f, t),
                1.0f
            );
            
            chunk->vertices.push_back(vertex);
        }
    }
    
    // Generate indices
    for (u32 i = 0; i < resolution - 1; i++) {
        for (u32 j = 0; j < resolution - 1; j++) {
            u32 topLeft = i * resolution + j;
            u32 topRight = i * resolution + (j + 1);
            u32 bottomLeft = (i + 1) * resolution + j;
            u32 bottomRight = (i + 1) * resolution + (j + 1);
            
            chunk->indices.push_back(topLeft);
            chunk->indices.push_back(bottomLeft);
            chunk->indices.push_back(topRight);
            
            chunk->indices.push_back(topRight);
            chunk->indices.push_back(bottomLeft);
            chunk->indices.push_back(bottomRight);
        }
    }
    
    ComputeNormals(chunk);
}

void WorldGenerator::ComputeNormals(TerrainChunk* chunk) {
    // Initialize normals to zero
    for (auto& vertex : chunk->vertices) {
        vertex.normal = Vec3::ZERO();
    }
    
    // Compute face normals and accumulate
    for (size_t i = 0; i < chunk->indices.size(); i += 3) {
        u32 i1 = chunk->indices[i];
        u32 i2 = chunk->indices[i + 1];
        u32 i3 = chunk->indices[i + 2];
        
        Vec3 v1 = chunk->vertices[i1].position;
        Vec3 v2 = chunk->vertices[i2].position;
        Vec3 v3 = chunk->vertices[i3].position;
        
        Vec3 edge1 = v2 - v1;
        Vec3 edge2 = v3 - v1;
        Vec3 faceNormal = edge1.Cross(edge2).Normalized();
        
        chunk->vertices[i1].normal += faceNormal;
        chunk->vertices[i2].normal += faceNormal;
        chunk->vertices[i3].normal += faceNormal;
    }
    
    // Normalize all normals
    for (auto& vertex : chunk->vertices) {
        vertex.normal = vertex.normal.Normalized();
    }
}

void WorldGenerator::GenerateStructures(TerrainChunk* chunk) {
    const auto& props = m_biomeProperties[static_cast<u32>(chunk->biomeMap[0])];
    
    if (props.structureDensity > 0.0f) {
        // Generate ruins based on biome
        switch (chunk->biomeMap[0]) {
            case BiomeType::DERELICT_CITY:
                GenerateRuins(chunk);
                break;
            case BiomeType::ALIEN_TERRAFORMING:
            case BiomeType::ALIEN_STRUCTURE:
                GenerateAlienStructures(chunk);
                break;
            case BiomeType::WAR_MACHINE_GRAVEYARD:
                GenerateWrecks(chunk);
                break;
            default:
                break;
        }
    }
}

void WorldGenerator::GenerateRuins(TerrainChunk* chunk) {
    // Generate ruined buildings
    i32 buildingCount = static_cast<i32>(m_random.Range(5, 20) * 
        m_biomeProperties[static_cast<u32>(chunk->biomeMap[0])].structureDensity);
    
    for (i32 i = 0; i < buildingCount; i++) {
        f32 x = m_random.Range(0.0f, m_chunkSize);
        f32 z = m_random.Range(0.0f, m_chunkSize);
        
        // Find height at position
        u32 ix = static_cast<u32>((x / m_chunkSize) * (chunk->size - 1));
        u32 iz = static_cast<u32>((z / m_chunkSize) * (chunk->size - 1));
        f32 y = chunk->heightmap[ix * chunk->size + iz];
        
        // TODO: Add building instance to render queue
    }
}

void WorldGenerator::GenerateAlienStructures(TerrainChunk* chunk) {
    // Generate alien structures
    i32 structureCount = static_cast<i32>(m_random.Range(3, 10) *
        m_biomeProperties[static_cast<u32>(chunk->biomeMap[0])].structureDensity);
    
    for (i32 i = 0; i < structureCount; i++) {
        f32 x = m_random.Range(0.0f, m_chunkSize);
        f32 z = m_random.Range(0.0f, m_chunkSize);
        
        // TODO: Add alien structure instance to render queue
    }
}

void WorldGenerator::GenerateWrecks(TerrainChunk* chunk) {
    // Generate war machine wrecks
    i32 wreckCount = static_cast<i32>(m_random.Range(10, 30) *
        m_biomeProperties[static_cast<u32>(chunk->biomeMap[0])].structureDensity);
    
    for (i32 i = 0; i < wreckCount; i++) {
        f32 x = m_random.Range(0.0f, m_chunkSize);
        f32 z = m_random.Range(0.0f, m_chunkSize);
        
        // TODO: Add wreck instance to render queue
    }
}

void WorldGenerator::UpdateStreaming(const Vec3& playerPosition, f32 viewDistance) {
    // Calculate visible chunks
    i32 chunkX = static_cast<i32>(playerPosition.x / m_chunkSize);
    i32 chunkZ = static_cast<i32>(playerPosition.z / m_chunkSize);
    i32 chunksInView = static_cast<i32>(viewDistance / m_chunkSize) + 2;
    
    std::unordered_set<u32> visibleChunks;
    
    for (i32 x = -chunksInView; x <= chunksInView; x++) {
        for (i32 z = -chunksInView; z <= chunksInView; z++) {
            i32 cx = chunkX + x;
            i32 cz = chunkZ + z;
            
            if (cx >= 0 && cz >= 0 && 
                cx * m_chunkSize < m_worldSize && 
                cz * m_chunkSize < m_worldSize) {
                u32 key = GetChunkKey(cx, cz);
                visibleChunks.insert(key);
                
                // Generate chunk if not exists
                if (m_chunks.find(key) == m_chunks.end()) {
                    GenerateChunk(cx, cz);
                }
            }
        }
    }
    
    // Remove chunks that are no longer visible (with a margin)
    // TODO: Implement chunk unloading with LOD system
}

TerrainChunk* WorldGenerator::GetChunk(u32 x, u32 z) {
    u32 key = GetChunkKey(x, z);
    auto it = m_chunks.find(key);
    return it != m_chunks.end() ? &it->second : nullptr;
}

} // namespace Duality