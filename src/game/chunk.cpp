#include <glad/glad.h>
#include "chunk.h"
#include "game_config.h"
#include "block_factory.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "window.h"
#include "shader.h"
#include "world.h"
#include "chunk_pos_hash.h"
#include "chunk_terrain_generator.h"

Chunk::Chunk(ChunkPos pos, World* world, const ChunkTerrainGenerator& terrain_generator):
chunk_data_{pos}, world_{world}
{
    terrain_generator.GenerateChunkTerrain(chunk_data_, pos.x, pos.z);
}

Chunk::~Chunk()
{
    DeleteBuffers();
}

void
Chunk::GenerateMesh(std::unordered_map<ChunkPos, Chunk*, ChunkPosHash>& adjacent_chunk_map)
{
    std::lock_guard<std::mutex> lock(mesh_mutex_);
    ChunkPos pos = chunk_data_.GetPos();
    std::cout << "Chunk::GenerateMesh(): Generating mesh for chunk at (" << pos.x << ", " << pos.z << ")" << std::endl;

    vertices_vao_.clear();
    texture_layers_vao_.clear();
    ebo_.clear();

    int num_verticies = 0;
    for (int i = 0; i < CHUNK_WIDTH; i++) {
        for (int j = 0; j < CHUNK_HEIGHT; j++) {
            for (int k = 0; k < CHUNK_WIDTH; k++) {
                if (chunk_data_.GetBlock(i, j, k) == BlockEnum::BlockId::AIR) {
                    continue;
                }
                for (int block_face = 0; block_face < 6; block_face++) {
                    if (IsFaceVisible(glm::vec3(i, j, k), static_cast<BlockFace>(block_face), adjacent_chunk_map)) {
                        const Block* block = BlockFactory::GetBlock(chunk_data_.GetBlock(i, j, k));
                        num_verticies += block->AddVerticies(vertices_vao_, ebo_, num_verticies, static_cast<BlockFace>(block_face), glm::vec3(i, j, k));
                        block->AddTextureLayers(texture_layers_vao_, static_cast<BlockFace>(block_face));
                    }
                }
            }
        }
    }
}

void
Chunk::RenderChunk()
{
    //try to lock the mutex. If mutex is locked, skip rendering (mesh is being regenerated)
    if (!mesh_mutex_.try_lock()) {
        // std::cout << "Chunk::RenderChunk(): mesh_mutex_ is locked. Skipping render for chunk at (" << pos_.x << ", " << pos_.z << ")" << std::endl;
        return;
    }

    if (vertices_vao_.size() == 0 || texture_layers_vao_.size() == 0 || ebo_.size() == 0) {
        // std::cout << "Chunk::RenderChunk(): VAO or EBO empty. Skipping render for chunk at (" << pos_.x << ", " << pos_.z << ")" << std::endl;
        mesh_mutex_.unlock();
        return;
    }

    //send buffer data to GPU
    InitializeBuffers();

    //send model matrix to shader
    Window::GetActiveWindow()->GetShader().SetMat4("model", GetModelMatrix());

    //render
    glDrawElements(GL_TRIANGLES, ebo_.size(), GL_UNSIGNED_INT, 0);

    //unlock mutex
    mesh_mutex_.unlock();
}

void
Chunk::InitializeBuffers()
{
    if (!buffers_initialized_) {
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &pos_tex_VBO_);
        glGenBuffers(1, &tex_layers_VBO_);
        glGenBuffers(1, &EBO_);
        buffers_initialized_ = true;
    }

    glBindVertexArray(VAO_);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*ebo_.size(), &ebo_[0], GL_STATIC_DRAW);

    // POSITION + TEXURE COORDINATES VBO
    glBindBuffer(GL_ARRAY_BUFFER, pos_tex_VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices_vao_.size(), &vertices_vao_[0], GL_STATIC_DRAW); //set pointer to data
    
    glVertexAttribPointer(VERTEX_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // vertex position
    glEnableVertexAttribArray(VERTEX_POS_LOCATION);
    
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // texture coordinates
    glEnableVertexAttribArray(TEX_COORD_LOCATION);

    // TEXTURE LAYER VBO
    glBindBuffer(GL_ARRAY_BUFFER, tex_layers_VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int)*texture_layers_vao_.size(), &texture_layers_vao_[0], GL_STATIC_DRAW); //set pointer to data

    glVertexAttribIPointer(TEX_LAYER_LOCATION, 1, GL_INT, 1 * sizeof(int), (void*)0); // texture layer
    glEnableVertexAttribArray(TEX_LAYER_LOCATION);
}

void
Chunk::DeleteBuffers()
{
    if (!buffers_initialized_) {
        return;
    }
    glDeleteVertexArrays(1, &VAO_);
    glDeleteBuffers(1, &pos_tex_VBO_);
    glDeleteBuffers(1, &tex_layers_VBO_);
    glDeleteBuffers(1, &EBO_);
    VAO_ = pos_tex_VBO_ = tex_layers_VBO_ = EBO_ = 0;
    buffers_initialized_ = false;
}

bool
Chunk::IsFaceVisible(const glm::vec3& position, const BlockFace face, std::unordered_map<ChunkPos, Chunk*, ChunkPosHash>& adjacent_chunk_map) const
{
    const Block* neighbour_block;
    
    if (IsFaceOnChunkBorder(position, face)) { //get neighbour block if current block is on chunk border
        if (face == BlockFace::Y_POS || face == BlockFace::Y_NEG) {
            return true;
        }

        ChunkPos neighbour_chunk_pos{0, 0};
        int neighbour_block_x;
        int neighbour_block_z;

        ChunkPos cur_chunk_pos = chunk_data_.GetPos();
        switch (face) {
            case BlockFace::X_POS:
                neighbour_chunk_pos = cur_chunk_pos + ChunkPos{1, 0};
                neighbour_block_x = 0;
                neighbour_block_z = position.z;
                break;
            case BlockFace::X_NEG:
                neighbour_chunk_pos = cur_chunk_pos + ChunkPos{-1, 0};
                neighbour_block_x = CHUNK_WIDTH - 1;
                neighbour_block_z = position.z;
                break;
            case BlockFace::Z_POS:
                neighbour_chunk_pos = cur_chunk_pos + ChunkPos{0, 1};
                neighbour_block_x = position.x;
                neighbour_block_z = 0;
                break;
            case BlockFace::Z_NEG:
                neighbour_chunk_pos = cur_chunk_pos + ChunkPos{0, -1};
                neighbour_block_x = position.x;
                neighbour_block_z = CHUNK_WIDTH - 1;
                break;
        }

        if (adjacent_chunk_map.count(neighbour_chunk_pos) == 0) { //if neighbour chunk does not exist then do not render face
            return false;
        }

        neighbour_block = BlockFactory::GetBlock(adjacent_chunk_map.at(neighbour_chunk_pos)->chunk_data_.GetBlock(neighbour_block_x, position.y, neighbour_block_z));
    }
    else { //get neighbour block from current chunk
        switch (face) {
            case BlockFace::X_POS:
                neighbour_block = BlockFactory::GetBlock(chunk_data_.GetBlock(position.x + 1, position.y, position.z));
                break;
            case BlockFace::X_NEG:
                neighbour_block = BlockFactory::GetBlock(chunk_data_.GetBlock(position.x - 1, position.y, position.z));
                break;
            case BlockFace::Z_POS:
                neighbour_block = BlockFactory::GetBlock(chunk_data_.GetBlock(position.x, position.y, position.z + 1));
                break;
            case BlockFace::Z_NEG:
                neighbour_block = BlockFactory::GetBlock(chunk_data_.GetBlock(position.x, position.y, position.z - 1));
                break;
            case BlockFace::Y_POS:
                neighbour_block = BlockFactory::GetBlock(chunk_data_.GetBlock(position.x, position.y + 1, position.z));
                break;
            case BlockFace::Y_NEG:
                neighbour_block = BlockFactory::GetBlock(chunk_data_.GetBlock(position.x, position.y - 1, position.z));
                break;
        }
    }

    //if neighbour block is nullptr then it's air (transparent)
    if (!neighbour_block || neighbour_block->GetOpacity() != BlockEnum::BlockOpacity::SOLID) {
        return true;
    }

    return false;
}

bool
Chunk::IsFaceOnChunkBorder(const glm::vec3& position, const BlockFace face) const
{
    if (position.x == 0 && face == BlockFace::X_NEG) {
        return true;
    }
    else if (position.x == CHUNK_WIDTH - 1 && face == BlockFace::X_POS) {
        return true;
    }
    else if (position.y == 0 && face == BlockFace::Y_NEG) {
        return true;
    }
    else if (position.y == CHUNK_HEIGHT - 1 && face == BlockFace::Y_POS) {
        return true;
    }
    else if (position.z == 0 && face == BlockFace::Z_NEG) {
        return true;
    }
    else if (position.z == CHUNK_WIDTH - 1 && face == BlockFace::Z_POS) {
        return true;
    }

    return false;
}

glm::mat4
Chunk::GetModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    ChunkPos pos = chunk_data_.GetPos();
    model = glm::translate(model, glm::vec3(pos.x * CHUNK_WIDTH, 0.0f, pos.z * CHUNK_WIDTH));
    return model;
}
