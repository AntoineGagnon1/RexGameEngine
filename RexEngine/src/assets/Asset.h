#pragma once

#include <filesystem>
#include <memory>
#include "../core/Guid.h"

namespace RexEngine
{
	template <typename T>
	concept IsAsset = requires(T t, const std::filesystem::path& p)
	{
		{ t.LoadFromAssetFile(p) } -> std::same_as<std::shared_ptr<T>>;
	};

	// To make an asset (the Shader class for example), you must have a 
	// member function like : std::shared_ptr<Type> LoadFromAssetFile(const std::filesystem::path&)
	// this function takes a path to the .asset metadata file. the path to the original asset is 
	// the metadata path - the .asset.
	template<IsAsset T>
	class Asset
	{
	public:
		inline static const std::string FileExtension = ".asset";

	public:
		Asset()
			: m_guid(Guid::Empty), m_asset(nullptr)
		{}

		inline static Asset<T> LoadFromAssetFile(const Guid& guid, std::filesystem::path& path)
		{
			Asset<T> asset;

			asset.m_guid = guid;
			asset.m_asset = T::LoadFromAssetFile(path);

			return asset;
		}

	private:

		Guid m_guid;
		std::shared_ptr<T> m_asset;
	};
}