#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "../core/Log.h"
#include "../events/EngineEvents.h"
#include "Texture.h"
#include "RenderApi.h"

namespace RexEngine
{
	// Used to manage which texture is in each texture slot
	class TextureManager
	{
	public:

		// Call this once before the first GetTextureSlot() call for each shader
		inline static void StartShader()
		{
			for (auto& slot : s_slots)
			{
				slot.usedThisShader = false;
			}
		}

		// All the slots returned by GetTextureSlot() in the same "Shader" (between 2 StartShader() calls)
		// will stay valid, any slot used in another "Shader" can change
		// This will bind the texture to the slot automatically, target is only used to bind
		inline static int GetTextureSlot(RenderApi::TextureID texture, RenderApi::TextureTarget target)
		{
			if (!texture)
				return 0;

			// Check if this texture is already bound to a slot
			if (auto slot = s_textureSlot.find(texture); slot != s_textureSlot.end())
			{
				s_slots[slot->second].usedThisShader = true;
				return slot->second;
			}

			// Find a not used slot to use
			// TODO : a smarter way to find the slot based on usage
			for (int i = 0; i < s_slots.size(); i++)
			{
				if (s_slots[i].usedThisShader == false)
				{
					// Remove the old
					s_textureSlot.erase(s_slots[i].texture);

					// Add the new
					s_textureSlot.insert({texture, i});
					s_slots[i].texture = texture;
					s_slots[i].usedThisShader = true;

					// Set the slot as active
					RenderApi::SetActiveTexture(i);
					RenderApi::BindTexture(texture, target);
					return i;
				}
			}

			// No slot left
			RE_LOG_ERROR("Not enough texture slots !");
			return 0;
		}

	private:

		inline static void Init()
		{
			// Get the number of slots on this machine
			int count = RenderApi::GetTextureSlotCount();

			s_slots.reserve(count);
			for (int i = 0; i < count; i++)
			{
				s_slots.push_back(Slot());
			}
		}

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStart().Register<&TextureManager::Init>();
		});

	private:
		struct Slot
		{
			RenderApi::TextureID texture;
			bool usedThisShader = false; // is this texture used by the current shader (if true dont replace it)
		};

		inline static std::vector<Slot> s_slots;
		inline static std::unordered_map<RenderApi::TextureID, int> s_textureSlot;
	};
}