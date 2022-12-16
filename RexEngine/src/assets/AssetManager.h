#pragma once

#include <unordered_map>
#include <any>
#include <fstream>
#include <filesystem>
#include <memory>

#include "../core/Guid.h"
#include "../core/Serialization.h"
#include "../events/EngineEvents.h"

namespace RexEngine
{
	// To make an asset (the Shader class for example), you must have these
	// member functions : 
	// 
	// template<typename Archive>
	// static std::shared_ptr<Type> LoadFromAssetFile(Guid assetGuid, const Archive& metaDataArchive, std::istream& assetFile)
	// 
	// template<typename Archive>
	// void SaveToAssetFile(Archive& metaDataArchive)
	// 
	// the path to the original asset is 
	// the metadata path - the .asset
	template<typename T>
	class Asset
	{
	public:
		inline static const std::string FileExtension = ".asset";

	public:
		Asset()
			: m_guid(Guid::Empty), m_asset(nullptr)
		{}


		// The save/load functions are not called by the AssetManager, they are only used when load/saving Components with Assets
		template<class Archive>
		void save(Archive& archive) const; // Definition at the end of the file because it needs the AssetEditor

		template<class Archive>
		void load(Archive& archive); // Definition at the end of the file because it needs the AssetEditor

		Guid GetAssetGuid() const { return m_guid; }
		void SetAssetGuid(const Guid& guid) { m_guid = guid; }

		Asset& operator=(std::shared_ptr<T> from)
		{
			m_asset = from;
			return *this;
		}

		Asset& operator=(Asset<T> from)
		{
			m_asset = from.m_asset;
			m_guid = from.m_guid;
			return *this;
		}

		operator std::shared_ptr<T>() const { return m_asset; }

		// Use the underlying asset directly using ->
		const T* operator->() const  { return m_asset.get(); }
		T* operator->()  { return m_asset.get(); }

		// Check if an asset is empty
		operator bool() const { return m_asset != nullptr; }

	private:
		friend class AssetManager;

		Guid m_guid;
		std::shared_ptr<T> m_asset;
	};


	class AssetManager
	{
	public:

		// Will load the asset if needed, but wont add the path into the registry
		// Will return an invalid asset if the asset does not exist
		template<typename T>
		inline static Asset<T> GetAsset(const Guid& guid)
		{
			auto asset = s_assets.find(guid);
			if (asset != s_assets.end()) // already loaded
				return std::any_cast<Asset<T>>(asset->second);

			// Load the asset
			auto path = s_registry.find(guid);
			if (path == s_registry.end())
				return Asset<T>(); // Not found
			
			// Load the asset from the file
			Asset<T> a;
			std::ifstream metaDataFile(path->second);
			std::ifstream assetFile(path->second.string().substr(0, path->second.string().length() - 6)); // Remove the .asset part

			if (!metaDataFile.is_open() || !assetFile.is_open())
				return Asset<T>(); // Failed

			JsonDeserializer metaDataArchive(metaDataFile);
			a.SetAssetGuid(guid);
			a.m_asset = T::LoadFromAssetFile(guid, metaDataArchive, assetFile);

			s_assets[guid] = a;

			return a;
		}

		// Will return an empty guid if no asset with the specified path was found
		// path is the path of this asset (NO .asset extension)
		static Guid GetAssetGuidFromPath(const std::filesystem::path& path);

		// Returns the .asset file
		// Will return an empty path if no asset with the specified guid was found
		static std::filesystem::path GetAssetPathFromGuid(const Guid& guid);

		// Save the asset to the .asset file, returns false if the file could not be opened or if the asset is not loaded
		template<typename T>
		inline static bool SaveAsset(const Guid& guid)
		{
			// Get the asset
			auto asset = s_assets.find(guid);
			if (asset == s_assets.end()) // Not loaded
				return false;

			// Get the path
			auto path = s_registry.find(guid);
			if (path == s_registry.end())
				return false; // No path found

			// Load the asset from the file
			std::ofstream metaDataFile(path->second, std::ios::trunc);

			if (!metaDataFile.is_open())
				return false; // Failed to open the file

			{
				JsonSerializer metaDataArchive(metaDataFile);
				std::any_cast<Asset<T>>(asset->second).m_asset->SaveToAssetFile(metaDataArchive);
			}

			if (metaDataFile.cur == 1)
			{ // If the file is still empty, add a json node
				metaDataFile << "{}";
			}

			return true;
		}

		// Add the asset to the registry, returns false if the file could not be opened/created
		// assetPath should be the path to the asset file, NOT to the metadata file
		// This will remove any existing assets with this guid
		template<typename T>
		inline static bool AddAsset(const Guid& guid, const std::filesystem::path& assetPath)
		{
			// Create or empty the .asset file
			auto fullPath = assetPath.string() + Asset<int>::FileExtension;
			std::ofstream assetFile(fullPath, std::ios::trunc);
			assetFile << "{}"; // Empty json object (JsonSerializer)

			s_registry[guid] = fullPath;

			// Write the registry
			std::ofstream file(s_registryPath, std::ios::trunc);
			if (!file.is_open())
				return false;

			JsonSerializer archive(file);
			archive(CUSTOM_NAME(s_registry, "Paths"));
			return true;
		}

		// Change the registry in use, returns false if the file could not be opened
		static bool SetRegistry(const std::filesystem::path& path);

		// Create an empty registry at the path, returns false if the file could not be opened
		static bool CreateRegistry(const std::filesystem::path& path);

	private:

		// Clear and delete all the assets
		// this should be called on EngineEvent::OnEngineStop
		inline static void Clear()
		{
			s_assets.clear();
			s_registry.clear();
			s_registryPath = "";
		}

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStop().Register<&AssetManager::Clear>();
		});

		inline static std::filesystem::path s_registryPath;

		// Guid of the asset, path to the .asset metadata file
		inline static std::unordered_map<Guid, std::filesystem::path> s_registry;

		// Guid of the asset, Asset<T>
		inline static std::unordered_map<Guid, std::any> s_assets;
	};


	template<typename T>
	template<class Archive>
	void Asset<T>::save(Archive& archive) const
	{
		archive(CUSTOM_NAME(m_guid, "Guid"));
		AssetManager::SaveAsset<T>(m_guid);
	}

	template<typename T>
	template<class Archive>
	inline void Asset<T>::load(Archive& archive)
	{
		archive(CUSTOM_NAME(m_guid, "Guid"));
		m_asset = AssetManager::GetAsset<T>(m_guid).m_asset;
	}

}