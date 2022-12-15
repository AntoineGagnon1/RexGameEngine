#include "REPch.h"
#include "AssetTypes.h"

#include "rendering/Shader.h"
#include "rendering/Mesh.h"

namespace RexEngine
{
	std::vector<AssetType> AssetTypes::GetDefaultTypes()
	{
		std::vector<AssetType> types = {
			{"Shader", { ".shader" }, typeid(Shader)},
			{ "Mesh", {".obj"}, typeid(Mesh)}
		};

		return types;
	}

	std::vector<AssetType> AssetTypes::s_assetTypes = AssetTypes::GetDefaultTypes();

	void AssetTypes::RegisterAssetType(const AssetType& type)
	{
		if (type.Empty())
			return;

		if (!GetAssetTypeFromName(type.name).Empty())
		{
			RE_LOG_ERROR("AssetType {} already registered !", type.name);
			return;
		}

		for (auto& t : s_assetTypes)
		{
			if (type.type == t.type)
			{
				RE_LOG_ERROR("AssetType {} already registered !", type.name);
				return;
			}
		}

		for (auto& extension : type.extensions)
		{
			if (!GetAssetTypeFromExtension(extension).Empty())
			{
				RE_LOG_ERROR("AssetType {} already registered !", extension);
				return;
			}
		}

		s_assetTypes.push_back(type);
	}

	AssetType AssetTypes::GetAssetTypeFromName(const std::string& name)
	{
		for (auto& type : s_assetTypes)
		{
			if (type.name == name)
				return type;
		}

		return AssetType();
	}

	AssetType AssetTypes::GetAssetTypeFromExtension(const std::string& extension)
	{
		for (auto& type : s_assetTypes)
		{
			for (auto& ex : type.extensions) 
			{
				if (ex == extension)
					return type;
			}
		}

		return AssetType();
	}
}