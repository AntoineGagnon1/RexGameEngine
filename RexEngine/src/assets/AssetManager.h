#pragma once

#include <unordered_map>
#include <any>
#include <fstream>

#include "Asset.h"
#include "../core/Guid.h"
#include "../core/Serialization.h"

namespace RexEngine
{
	class AssetManager
	{
	public:

		// Will load the asset if needed
		// Will return an invalid asset if the asset does not exist
		template<IsAsset T>
		inline static Asset<T> GetAsset(const Guid& guid)
		{
			auto asset = s_assets.find(guid);
			if (asset != s_assets.end()) // already loaded
				return std::any_cast<Asset<T>>(asset->second);

			// Load the asset
			auto path = s_registry.find(guid);
			if (path == s_registry.end())
				return Asset<T>(); // Not found
			
			Asset<T> a = Asset<T>::LoadFromAssetFile(guid, path->second);
			s_assets[guid] = a;

			return a;
		}

		// Add the asset to the registry, returns false if the file could not be opened
		// assetPath should be the path to the .asset file
		// This will remove any existing assets with this guid
		inline static bool AddAsset(const Guid& guid, const std::filesystem::path& assetPath)
		{
			s_registry[guid] = assetPath;

			// Write the registry
			std::ofstream file(s_registryPath, std::ios::trunc);
			if (!file.is_open())
				return false;

			JsonSerializer archive(file);
			archive(CUSTOM_NAME(s_registry, "Paths"));
			return true;
		}

		// Change the registry in use, returns false if the file could not be opened
		inline static bool SetRegistry(const std::filesystem::path& path)
		{
			s_registry.clear();
			s_assets.clear();

			s_registryPath = path;

			// Read the registry
			std::ifstream file(s_registryPath);
			if (!file.is_open())
				return false;

			JsonDeserializer archive(file);
			archive(CUSTOM_NAME(s_registry, "Paths"));
			return true;
		}

		// Create an empty registry at the path, returns false if the file could not be opened
		inline static bool CreateRegistry(const std::filesystem::path& path)
		{
			std::ofstream file(path, std::ios::trunc);
			if (!file.is_open())
				return false;

			JsonSerializer archive(file);
			std::unordered_map<Guid, std::filesystem::path> emptyRegistry;
			archive(CUSTOM_NAME(emptyRegistry, "Paths"));
			return true;
		}

	private:

		inline static std::filesystem::path s_registryPath;

		// Guid of the asset, path to the .asset metadata file
		inline static std::unordered_map<Guid, std::filesystem::path> s_registry;

		// Guid of the asset, Asset<T>
		inline static std::unordered_map<Guid, std::any> s_assets;
	};
}