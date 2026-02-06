#include "d3d11/GraphicsPipeline.hpp"

namespace core {
    InputLayoutCache::InputLayoutCache() = default;
    InputLayoutCache::InputLayoutCache(const InputLayoutCache& other) {
        this->operator=(other);
    }
    InputLayoutCache::InputLayoutCache(InputLayoutCache&& other) {
        this->operator=(std::move(other));
    }
    InputLayoutCache::~InputLayoutCache() = default;

    InputLayoutCache& InputLayoutCache::operator=(const InputLayoutCache& other) {
        if (this == &other) {
            return *this;
        }
        assert(other.semantic_names.size() == other.input_elements.size());
        semantic_names = other.semantic_names;
        input_elements = other.input_elements;
        for (size_t i = 0; i < input_elements.size(); i += 1) {
            input_elements[i].SemanticName = semantic_names[i].c_str();
        }
        shader_byte_code = other.shader_byte_code;
        hash_value = other.hash_value;
        return *this;
    }
    InputLayoutCache& InputLayoutCache::operator=(InputLayoutCache&& other) {
        if (this == &other) {
            return *this;
        }
        assert(other.semantic_names.size() == other.input_elements.size());
        semantic_names = std::move(other.semantic_names);
        input_elements = std::move(other.input_elements);
        shader_byte_code = std::move(other.shader_byte_code);
        hash_value = other.hash_value;
        return *this;
    }

    void InputLayoutCache::initialize(
        const D3D11_INPUT_ELEMENT_DESC* const p_input_elements, const UINT input_element_count,
        const void* const p_shader_byte_code, const SIZE_T shader_byte_code_length
    ) {
        assert(p_input_elements != nullptr);
        assert(input_element_count != 0);
        assert(p_shader_byte_code != nullptr);
        assert(shader_byte_code_length != 0);
        semantic_names.resize(input_element_count);
        input_elements.resize(input_element_count);
        for (uint32_t i = 0; i < input_element_count; i += 1) {
            assert(p_input_elements[i].SemanticName != nullptr);
            input_elements[i] = p_input_elements[i];
            semantic_names[i].assign(p_input_elements[i].SemanticName);
            input_elements[i].SemanticName = semantic_names[i].c_str();
        }
        shader_byte_code.resize(shader_byte_code_length);
        std::memcpy(shader_byte_code.data(), p_shader_byte_code, shader_byte_code_length);
    }

    size_t InputLayoutCacheOp::operator()(const InputLayoutCache& s) const noexcept {
        return s.hash_value;
    }

    bool InputLayoutCacheOp::operator()(const InputLayoutCache& a, const InputLayoutCache& b) const noexcept {
        if (a.hash_value != b.hash_value) {
            return false;
        }
        if (a.input_elements.size() != b.input_elements.size()) {
            return false;
        }
        for (size_t i = 0; i < a.input_elements.size(); i += 1) {
            const auto& a_semantic_name = a.semantic_names[i];
            const auto& a_input_element = a.input_elements[i];
            const auto& b_semantic_name = b.semantic_names[i];
            const auto& b_input_element = b.input_elements[i];
            if (a_semantic_name != b_semantic_name) {
                return false;
            }
            if (std::memcmp(&a_input_element.SemanticIndex, &b_input_element.SemanticIndex, sizeof(a_input_element) - sizeof(a_input_element.SemanticName)) != 0) {
                return false;
            }
        }
        if (a.shader_byte_code.size() != b.shader_byte_code.size()) {
            return false;
        }
        if (std::memcmp(a.shader_byte_code.data(), b.shader_byte_code.data(), a.shader_byte_code.size()) != 0) {
            return false;
        }
        return true;
    }

    void GraphicsPipelineCache::hash(InputLayoutCache* const input_layout_cache) {
        XXH3_64bits_reset(m_hash_state);
        const auto& semantic_names = input_layout_cache->semantic_names;
        const auto& input_elements = input_layout_cache->input_elements;
        assert(semantic_names.size() == input_elements.size());
        for (size_t i = 0; i < input_elements.size(); i += 1) {
            const auto& semantic_name = semantic_names[i];
            const auto& input_element = input_elements[i];
            XXH3_64bits_update(m_hash_state, semantic_name.c_str(), semantic_name.size());
            XXH3_64bits_update(m_hash_state, &input_element.SemanticIndex, sizeof(input_element) - sizeof(input_element.SemanticName));
        }
        const auto& shader_byte_code = input_layout_cache->shader_byte_code;
        XXH3_64bits_update(m_hash_state, shader_byte_code.data(), shader_byte_code.size());
        input_layout_cache->hash_value = XXH3_64bits_digest(m_hash_state);
    }
}
