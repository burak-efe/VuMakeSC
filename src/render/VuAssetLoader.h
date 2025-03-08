#pragma once

#include <filesystem>
#include <iostream>

#include "Common.h"
#include "VuMesh.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include "VuResourceManager.h"
#include "VuTypes.h"

namespace Vu {

    template<typename T>
    struct VuAssetRef {
        const char* path;
        VkBool32    isLoaded;
        VuHandle<T> handle;
    };


    struct VuAssetLoader {

        static void LoadGltf(const std::filesystem::path& path, VuMesh& dstMesh) {

            fastgltf::Parser parser;

            auto data = fastgltf::GltfDataBuffer::FromPath(path);

            if (data.error() != fastgltf::Error::None) {
                std::cout << "gltf file cannot be loaded!" << "\n";
            }

            auto asset = parser.loadGltf(
                data.get(), path.parent_path(), fastgltf::Options::LoadExternalBuffers);
            if (auto error = asset.error(); error != fastgltf::Error::None) {
                std::cout << "Some error occurred while reading the buffer, parsing the JSON, or validating the data." << "\n";
            }

            auto mesh      = asset.get().meshes.at(0);
            auto prims     = mesh.primitives;
            auto primitive = prims.at(0);

            auto parentPath = path.parent_path();


            dstMesh.vertexBuffer.createHandle();
            dstMesh.indexBuffer.createHandle();


            //Indices

            if (!primitive.indicesAccessor.has_value()) {
                std::cout << "Primitive index accessor has not been set!" << "\n";
            }
            auto&  indexAccesor = asset->accessors[primitive.indicesAccessor.value()];
            uint64 indexCount   = indexAccesor.count;
            dstMesh.indexBuffer.get()->init({
                .length = indexCount,
                .strideInBytes = sizeof(uint32),
                .usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            });
            dstMesh.indexBuffer.get()->map();
            auto              indexSpanByte = dstMesh.indexBuffer.get()->getSpan(0, indexCount * sizeof(uint32));
            std::span<uint32> indexSpan     = std::span(reinterpret_cast<uint32 *>(indexSpanByte.data()), indexCount);
            fastgltf::iterateAccessorWithIndex<uint32>(
                asset.get(), indexAccesor,
                [&](uint32 index, std::size_t idx) { indexSpan[idx] = index; }
            );
            dstMesh.indexBuffer.get()->unmap();


            //Position
            fastgltf::Attribute* positionIt       = primitive.findAttribute("POSITION");
            fastgltf::Accessor&  positionAccessor = asset->accessors[positionIt->accessorIndex];

            dstMesh.vertexCount = static_cast<uint32>(positionAccessor.count);
            dstMesh.vertexBuffer.get()->init({
                .length = dstMesh.vertexCount * dstMesh.totalAttributesSizePerVertex(),
                .strideInBytes = 1U,
                .usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            });
            VuResourceManager::registerStorageBuffer(dstMesh.vertexBuffer.index, *dstMesh.vertexBuffer.get());
            dstMesh.vertexBuffer.get()->map();

            std::span<uint8>  vertexSpanByte = dstMesh.vertexBuffer.get()->getSpan(0, sizeof(float3) * dstMesh.vertexCount);
            std::span<float3> vertexSpan     = std::span(reinterpret_cast<float3 *>(vertexSpanByte.data()), dstMesh.vertexCount);

            std::span<uint8> normalSpanByte = dstMesh.vertexBuffer.get()->getSpan(dstMesh.getNormalOffsetAsByte(),
                                                                                  sizeof(float3) * dstMesh.vertexCount);
            std::span<float3> normalSpan = std::span(reinterpret_cast<float3 *>(normalSpanByte.data()), dstMesh.vertexCount);

            std::span<uint8> tangentSpanByte = dstMesh.vertexBuffer.get()->getSpan(dstMesh.getTangentOffsetAsByte(),
                                                                                   sizeof(float4) * dstMesh.vertexCount);
            std::span<float4> tangentSpan = std::span(reinterpret_cast<float4 *>(tangentSpanByte.data()), dstMesh.vertexCount);

            std::span<uint8> uvSpanByte = dstMesh.vertexBuffer.get()->getSpan(dstMesh.getUV_OffsetAsByte(),
                                                                              sizeof(float2) * dstMesh.vertexCount);
            std::span<float2> uvSpan = std::span(reinterpret_cast<float2 *>(uvSpanByte.data()), dstMesh.vertexCount);

            //pos
            {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(
                    asset.get(), positionAccessor,
                    [&](const glm::vec3 pos,const std::size_t idx) { vertexSpan[idx] = pos; }
                );
            }

            //normal
            {
                auto* normalIt       = primitive.findAttribute("NORMAL");
                auto& normalAccessor = asset->accessors[normalIt->accessorIndex];

                fastgltf::iterateAccessorWithIndex<glm::vec3>(
                    asset.get(), normalAccessor,
                    [&](const glm::vec3 normal,const std::size_t idx) { normalSpan[idx] = normal; }
                );

            }
            //uv
            {
                auto* uvIter     = primitive.findAttribute("TEXCOORD_0");
                auto& uvAccessor = asset->accessors[uvIter->accessorIndex];

                fastgltf::iterateAccessorWithIndex<glm::vec2>(
                    asset.get(), uvAccessor,
                    [&uvSpan](const glm::vec2 uv, const std::size_t idx) { uvSpan[idx] = uv; }
                );
            }

            //tangent
            {
                auto* tangentIt = primitive.findAttribute("TANGENT");

                fastgltf::Accessor& tangentAccessor    = asset->accessors[tangentIt->accessorIndex];
                auto                tangentbufferIndex = tangentAccessor.bufferViewIndex.value();
                if (tangentbufferIndex == 0U && tangentAccessor.byteOffset == 0U) {
                    std::cout << "Gltf file has no tangents" << std::endl;

                    Vu::VuMesh::calculateTangents(indexSpan, vertexSpan, normalSpan, uvSpan, tangentSpan);

                } else {
                    fastgltf::iterateAccessorWithIndex<float4>(
                        asset.get(), tangentAccessor,
                        [&tangentSpan](const float4 tangent, const std::size_t idx) { tangentSpan[idx] = tangent; }
                    );
                }
            }


            dstMesh.vertexBuffer.get()->unmap();
        }
    };
}
