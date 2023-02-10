#pragma once
#include <typeindex>
#include <unordered_map>

#include "../core/Log.h"
#include "RenderApi.h"

namespace RexEngine
{
	class UniformBlock
	{
	public:
		UniformBlock(int location, std::type_index type)
			: m_location(location), m_type(type), m_buffer(RenderApi::InvalidBufferID)
		{ }

		auto GetLocation() const { return m_location; }
		auto GetBufferID() const { return m_buffer; }

		template<typename T>
		void SetData(const T& data) const
		{
			RE_ASSERT(typeid(T) == m_type, "Invalid data type for uniform block : {}", typeid(T).name());

			RenderApi::SubBufferData(m_buffer, RenderApi::BufferType::Uniforms, 0, sizeof(T), &data);
		}

	private:
		int m_location;
		RenderApi::BufferID m_buffer;
		std::type_index m_type;

		friend class UniformBlocks;
	};

	class UniformBlocks
	{
	public:
		
		// Needs to be called once before GetBlock()
		// Returns the location of the block
		template<typename T>
		static int ReserveBlock(const std::string& name)
		{
			// Dont allocate the GPU buffer now, because Opengl might not be init yet
			const int location = static_cast<int>(s_blocks.size());
			s_blocks.insert({ name, UniformBlock(location, typeid(T)) });
			return location;
		}


		template<typename T>
		static UniformBlock& GetBlock(const std::string& name)
		{
			RE_ASSERT(s_blocks.contains(name), "No uniform named {}", name);

			auto& block = s_blocks.at(name);

			if (block.m_buffer == RenderApi::InvalidBufferID)
			{
				block.m_buffer = RenderApi::MakeBuffer();
				RenderApi::SetBufferData(block.m_buffer, RenderApi::BufferType::Uniforms, RenderApi::BufferMode::Dynamic, nullptr, sizeof(T));
				RenderApi::BindBufferBase(block.m_buffer, block.GetLocation());
			}

			return s_blocks.at(name);
		}

	private:
		inline static std::unordered_map<std::string, UniformBlock> s_blocks;
	};
}