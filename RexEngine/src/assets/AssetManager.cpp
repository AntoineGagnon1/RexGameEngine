#include "REPch.h"
#include "AssetManager.h"

namespace RexEngine
{
	Guid AssetManager::GetAssetGuidFromPath(std::filesystem::path path)
	{
		AddMetaExtension(path);
		for (auto& pair : s_registry)
		{
			std::error_code code;
			if (std::filesystem::equivalent(pair.second, path, code))
				return pair.first;
		}

		return Guid::Empty;
	}

	std::filesystem::path AssetManager::GetAssetMetaPathFromGuid(const Guid& guid)
	{
		if (s_registry.contains(guid))
			return s_registry[guid];
		else
			return "";
	}

	std::filesystem::path AssetManager::GetAssetPathFromGuid(const Guid& guid)
	{
		return GetAssetMetaPathFromGuid(guid).replace_extension(""); // remove the .asset
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