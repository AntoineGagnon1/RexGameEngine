#pragma once

#include <string>
#include <vector>

#include <typeinfo>

namespace RexEngine
{
	// Info on an asset type
	struct AssetType
	{
		std::string name; // ex : Shader File
		std::vector<std::string> extensions; // ex : .shader
		std::type_index type = typeid(void); // ex : typeid(Shader)

		// Is this asset type empty (invalid) ?
		bool Empty() const
		{
			return name.empty() || extensions.empty() || type == typeid(void);
		}
	};

	// All assets type must be registered here to work properly
	class AssetTypes
	{
	public:

		static void RegisterAssetType(const AssetType& type);

		static AssetType GetAssetTypeFromName(const std::string& name);
		static AssetType GetAssetTypeFromExtension(const std::string& extension);

		template<typename T>
		inline static AssetType GetAssetType()
		{
			for (auto& type : s_assetTypes)
			{
				if (type.type == typeid(T))
					return type;
			}

			return AssetType();
		}

	private:

		static std::vector<AssetType> GetDefaultTypes();

		static std::vector<AssetType> s_assetTypes;
	};
}