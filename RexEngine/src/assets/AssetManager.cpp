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
			
			if (code)
			{ // An error occured (file does not exist ?), try a different comparaison
				auto fullpath1 = std::filesystem::weakly_canonical(std::filesystem::absolute(path));
				auto fullpath2 = std::filesystem::weakly_canonical(std::filesystem::absolute(pair.second));

				if (fullpath1 == fullpath2)
					return pair.first;
			}
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
				if (entry.path().has_extension() && entry.path().extension() != AssetFileExtension)
				{
					auto metaPath = entry.path();
					AddMetaExtension(metaPath);

					Guid guid;
					if (std::filesystem::exists(metaPath))
					{
						// Read the guid
						std::ifstream file(metaPath);
						JsonDeserializer archive(file);

						archive(CUSTOM_NAME(guid, "AssetGuid"));
					}
					else
					{
						guid = Guid::Generate(); // The asset has no meta file, generate a guid
					}

					s_registry.insert({ guid, metaPath });
				}
			}
		}
	}
}