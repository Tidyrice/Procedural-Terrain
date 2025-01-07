#ifndef WORLD_H
#define WORLD_H

#include "chunk_pos.h"
#include "chunk_pos_hash.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include "game_config.h"
#include "ts_queue.h"

class Chunk;

class World {
    public:
        World(ChunkPos pos);
        ~World();

        void RenderWorld(); //renders the world around the current chunk
        void UpdateChunkPos(const ChunkPos& pos); //specifies new chunk player is in

    private:
        void CreateChunkThreaded();
        void GenerateChunkMeshThreaded();

        void AddVisibleChunksToCreationQueue();
        void RegenerateAdjacentChunkMeshes(const ChunkPos& pos);

        friend class Chunk; //allows Chunk class to access GetChunk()
        const Chunk* GetChunk(const ChunkPos& pos); //used by Chunk class
        std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunk_map_; //holds all active chunks
        std::recursive_mutex chunk_map_mutex_;
        
        ts_queue<ChunkPos> chunk_creation_queue_; //holds chunks that need to be created
        ts_queue<ChunkPos> chunk_mesh_queue_; //holds chunks that need to have their mesh generated

        ChunkPos current_chunk_pos_; //current chunk the player is in
        int renderDistance_ = RENDER_DISTANCE; //render distance in chunks

        bool terminate_threads_ = false;
};

#endif // WORLD_H
