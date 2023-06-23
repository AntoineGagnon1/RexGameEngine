#pragma once
#include "../assets/AssetManager.h"

#include <format>

namespace RexEngine
{
	class ScriptAsset
	{
	public:

		static void CreateNewScript(const std::filesystem::path& path, const std::string& projectName)
		{
			const auto className = path.filename().replace_extension("");
			std::ofstream stream(path);
			stream << std::format("using System;\n"
								"using System.Collections.Generic;\n"
								"using RexEngine;\n"
								"\n"
								"namespace {}\n"
								"{{\n"
								"	class {} : ScriptComponent\n"
								"	{{\n"
								"		void OnUpdate()\n"
								"        {{\n"
								"			\n"  
								"        }}\n"
								"	}}\n"
								"}}\n", projectName, className.string());
		}

		template<typename Archive>
		static std::shared_ptr<ScriptAsset> LoadFromAssetFile([[maybe_unused]]Guid assetGuid, [[maybe_unused]]Archive& metaDataArchive, [[maybe_unused]]std::istream& assetFile)
		{
			return std::make_shared<ScriptAsset>();
		}
	};
}