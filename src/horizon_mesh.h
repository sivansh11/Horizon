#ifndef HORIZON_MESH_H
#define HORIZON_MESH_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_buffer.h"

#include <glm/glm.hpp>

#include <memory>

namespace horizon {

struct Vertex {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
        return position == other.position &&
               color    == other.color    &&
               normal   == other.normal   &&
               uv       == other.uv;
    }
};

class Mesh {
public:
    Mesh(gfx::Device& deviceRef, std::vector<Vertex> *vertices, std::vector<uint32_t> *indices);
    ~Mesh();

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBUffer);

private:
    void createVertexBuffer(std::vector<Vertex> *vertices);
    void createIndexBuffer(std::vector<uint32_t> *indices);

private:
    gfx::Device& mDevice;

    std::unique_ptr<gfx::Buffer> vertexBuffer{nullptr};
    std::unique_ptr<gfx::Buffer> indexBuffer{nullptr};
};

} // namespace horizon


#endif