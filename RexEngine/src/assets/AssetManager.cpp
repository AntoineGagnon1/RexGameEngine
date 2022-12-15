#include "REPch.h"
#include "AssetManager.h"

namespace RexEngine
{
	Guid AssetManager::GetAssetGuidFromPath(const std::filesystem::path& path)
	{
		for (auto& pair : s_registry)
		{
			if (std::filesystem::equivalent(pair.second, path.string() + Asset<int>::FileExtension))
				return pair.first;
		}

		return Guid::Empty;
	}

	std::filesystem::path AssetManager::GetAssetPathFromGuid(const Guid& guid)
	{
		if (s_registry.contains(guid))
			return s_registry[guid];
		else
			return "";
	}

	bool AssetManager::SetRegistry(const std::filesystem::path& path)
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

	bool AssetManager::CreateRegistry(const std::filesystem::path& path)
	{
		std::ofstream file(path, std::ios::trunc);
		if (!file.is_open())
			return false;

		JsonSerializer archive(file);
		std::unordered_map<Guid, std::filesystem::path> emptyRegistry;
		archive(CUSTOM_NAME(emptyRegistry, "Paths"));
		return true;
	}
}