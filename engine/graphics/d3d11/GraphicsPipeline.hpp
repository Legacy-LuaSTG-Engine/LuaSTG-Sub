#pragma once
#include "core/GraphicsPipeline.hpp"
#include "xxhash.h"
#include "d3d11/pch.h"

namespace core {
    struct InputLayoutCache {
        std::vector<std::string> semantic_names;
        std::vector<D3D11_INPUT_ELEMENT_DESC> input_elements;
        std::vector<uint8_t> shader_byte_code;
        XXH64_hash_t hash_value{};

        InputLayoutCache();
        InputLayoutCache(const InputLayoutCache& other);
        InputLayoutCache(InputLayoutCache&& other);
        ~InputLayoutCache();

        InputLayoutCache& operator=(const InputLayoutCache& other);
        InputLayoutCache& operator=(InputLayoutCache&& other);

        void initialize(
            const D3D11_INPUT_ELEMENT_DESC* p_input_elements, UINT input_element_count,
            const void* p_shader_byte_code, SIZE_T shader_byte_code_length
        );
    };

    struct InputLayoutCacheOp {
        size_t operator()(const InputLayoutCache& s) const noexcept; // std::hash<InputLayoutCache>()
        bool operator()(const InputLayoutCache& a, const InputLayoutCache& b) const noexcept; // std::equal_to<InputLayoutCache>
    };

    class GraphicsPipelineCache {
    private:
        void hash(InputLayoutCache* input_layout_cache);

        XXH3_state_t* m_hash_state{};
        std::pmr::unsynchronized_pool_resource m_memory_resource;
        std::pmr::unordered_set<InputLayoutCache, InputLayoutCacheOp, InputLayoutCacheOp> m_input_layout_cache;

    };
}
