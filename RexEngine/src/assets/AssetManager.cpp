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

	void AssetManager::LoadRegistry(const std::filesystem::path& path)
	{
		s_registry.clear();
		s_assets.clear();
		LoadRegistryRecursive(path);
	}

	void AssetManager::LoadRegistryRecursive(const std::filesystem::path & path)
	{
		for (auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				LoadRegistryRecursive(entry.path());
			}
			else
			{ // File
				if (entry.path().has_extension() && entry.path().extension() == Asset<int>::FileExtension)
				{
					// Read the guid
					std::ifstream file(entry.path());
					JsonDeserializer archive(file);

					Guid guid;
					archive(CUSTOM_NAME(guid, "AssetGuid"));

					s_registry.insert({ guid, entry.path() });
				}
			}
		}
	}
}