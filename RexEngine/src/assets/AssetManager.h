#pragma once

#include <unordered_map>
#include <any>
#include <fstream>
#include <filesystem>
#include <memory>
#include <sstream>

#include "AssetTypes.h"
#include "../core/Guid.h"
#include "../core/Serialization.h"
#include "../core/EngineEvents.h"

namespace RexEngine
{
	// To make an asset (the Shader class for example), you must have these
	// member functions : 
	// 
	// template<typename Archive>
	// static std::shared_ptr<Type> LoadFromAssetFile(Guid assetGuid, Archive& metaDataArchive, std::istream& assetFile)
	// 
	// And : (these can be ommited if they are not needed (ie read-only assets))
	// template<typename Archive>
	// void SaveToAssetFile(Archive& metaDataArchive)
	// 
	// Or (WARNING : this version will empty the assetFile before passing it):
	// template<typename Archive>
	// void SaveToAssetFile(Archive& metaDataArchive, std::ostream& assetFile)
	// 
	// If the type requires MetaData :
	// this is used to create a valid MetaData file for types like textures 
	// template<typename Archive>
	// static void CreateMetaData(Archive& metaDataArchive)
	// 
	// the path to the original asset is 
	// the metadata path - the .asset
	template<typename T>
	class Asset
	{
	public:
		Asset()
			: m_guid(Guid::Empty), m_asset(nullptr)
		{}

		Asset(const Guid& guid, std::shared_ptr<T> asset) 
			: m_guid(guid), m_asset(std::make_shared<std::shared_ptr<T>>(asset))
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
			m_asset = std::make_shared<std::shared_ptr<T>>(from);
			return *this;
		}

		Asset& operator=(Asset<T> from)
		{
			m_asset = from.m_asset;
			m_guid = from.m_guid;
			return *this;
		}

		operator std::shared_ptr<T>() const { return *m_asset; }

		// Use the underlying asset directly using ->
		const T* operator->() const  { return m_asset->get(); }
		T* operator->()  { return m_asset->get(); }

		// Check if an asset is empty
		operator bool() const { return m_asset != nullptr; }

	private:
		friend class AssetManager;

		Guid m_guid;
		std::shared_ptr<std::shared_ptr<T>> m_asset; // Store a ptr to a ptr to make reload possibles
	};


	inline const std::string AssetFileExtension = ".asset";


	// Check for this : 
	// template<typename Archive>
	// void SaveToAssetFile(Archive& metaDataArchive)
	template<typename T, typename Archive>
	concept HasSaveMetaOnly = requires(T* asset, Archive a)
	{
		asset->SaveToAssetFile(a);
	};

	// Check for this :
	// template<typename Archive>
	// void SaveToAssetFile(Archive& metaDataArchive, std::ostream& assetFile)
	template<typename T, typename Archive>
	concept HasSaveMetaAndAsset = requires(T * asset, Archive a, std::ostream file)
	{
		asset->SaveToAssetFile(a, file);
	};

	// Check for this :
	// template<typename Archive>
	// static void CreateMetaData(Archive& metaDataArchive)
	template<typename T, typename Archive>
	concept HasCreateMeta = requires(Archive a)
	{
		T::CreateMetaData(a);
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

			// Generate the meta file if needed
			if (!std::filesystem::exists(path->second))
			{
				std::ofstream file(path->second);
				if (!file.is_open())
					return Asset<T>();

				JsonSerializer archive(file);
				// Add the guid
				archive(CUSTOM_NAME(Guid::Generate(), "AssetGuid"));

				// Try to use the metaData creation function
				if constexpr (HasCreateMeta<T, JsonSerializer>)
					T::CreateMetaData(archive);
			}

			// Load the asset from the file
			Asset<T> a;
			std::ifstream metaDataFile(path->second);
			bool bin = AssetTypes::GetAssetType<T>().binary;
			std::ifstream assetFile(path->second.string().substr(0, path->second.string().length() - 6), bin ? std::ios::binary : std::ios::in); // Remove the .asset part, std::ios::in used as a dummy

			if (!metaDataFile.is_open() || !assetFile.is_open())
				return Asset<T>(); // Failed

			JsonDeserializer metaDataArchive(metaDataFile);
			// Pop the guid
			Guid tempguid;
			metaDataArchive(CUSTOM_NAME(tempguid, "AssetGuid"));

			a.SetAssetGuid(guid);
			a.m_asset = std::make_shared<std::shared_ptr<T>>(T::LoadFromAssetFile(guid, metaDataArchive, assetFile));

			s_assets[guid] = a;

			return a;
		}

		template<typename T>
		inline static Asset<T> ReloadAsset(const Guid& guid)
		{
			SaveAsset<T>(guid);
			auto path = s_registry.find(guid);
			if (!s_assets.contains(guid) || path == s_registry.end())
				return Asset<T>();

			auto asset = std::any_cast<Asset<T>>(s_assets[guid]);
			(*asset.m_asset) = nullptr; // Unload


			// Load from the file
			std::ifstream metaDataFile(path->second);
			bool bin = AssetTypes::GetAssetType<T>().binary;
			std::ifstream assetFile(path->second.string().substr(0, path->second.string().length() - 6), bin ? std::ios::binary : std::ios::in); // Remove the .asset part, std::ios::in used as a dummy

			if (!metaDataFile.is_open() || !assetFile.is_open())
				return Asset<T>(); // Failed

			JsonDeserializer metaDataArchive(metaDataFile);
			// Pop the guid
			Guid tempguid;
			metaDataArchive(CUSTOM_NAME(tempguid, "AssetGuid"));

			(*asset.m_asset) = T::LoadFromAssetFile(guid, metaDataArchive, assetFile); // The reloaded asset

			return asset;
		}

		// Will return an empty guid if no asset with the specified path was found
		// path can inlude the .asset or not
		static Guid GetAssetGuidFromPath(std::filesystem::path path);

		// Returns the .asset file
		// Will return an empty path if no asset with the specified guid was found
		static std::filesystem::path GetAssetMetaPathFromGuid(const Guid& guid);

		// Returns the asset file (not .asset)
	 	// Will return an empty path if no asset with the specified guid was found
		static std::filesystem::path GetAssetPathFromGuid(const Guid& guid);

		// Save the asset to the .asset file, returns false if the file could not be opened or if the asset is not loaded
		template<typename T>
		inline static bool SaveAsset(const Guid& guid)
		{
			// Get the asset
			auto assetAny = s_assets.find(guid);
			if (assetAny == s_assets.end()) // Not loaded
				return false;

			// Get the path
			auto path = s_registry.find(guid);
			if (path == s_registry.end())
				return false; // No path found

			// Get the asset
			auto asset = std::any_cast<Asset<T>>(assetAny->second);

			return TrySaveAsset<T>(path->second, asset);
		}

		// Add the asset to the registry, returns false if the file could not be opened/created
		// assetPath can have the .asset extension or not
		// This will remove any existing assets with this guid or this path
		// if newAsset is empty the asset and .asset files wont be changed (leave empty for existing assets)
		template<typename T>
		inline static bool AddAsset(const Guid& guid, std::filesystem::path assetPath, Asset<T> newAsset = Asset<T>())
		{
			AddMetaExtension(assetPath);
			
			for (auto& [guid, path] : s_registry)
			{
				std::error_code code;
				if (std::filesystem::equivalent(path, assetPath, code))
				{ // Delete this entry
					s_registry.erase(guid);
					break;
				}
			}

			// Update the registry
			s_registry[guid] = assetPath;

			// Save the asset if needed
			if (newAsset)
			{
				if (!TrySaveAsset<T>(assetPath, newAsset))
					return false;
			}

			if (!std::filesystem::exists(assetPath)) // no .asset file either the creation failed or newAsset is empty
			{
				std::ofstream file(assetPath);
				if (!file.is_open())
					return false;
				
				JsonSerializer archive(file);
				// Add the guid
				archive(CUSTOM_NAME(guid, "AssetGuid"));

				// Try to use the metaData creation function
				if constexpr (HasCreateMeta<T, JsonSerializer>)
					T::CreateMetaData(archive);
			}

			return true;
		}

		// Load all the .asset files to get the guid, add each asset to the registry
		static void LoadRegistry(const std::filesystem::path& path);

	private:

		// add the .asset after the path
		inline static void AddMetaExtension(std::filesystem::path& path)
		{
			if (!path.has_extension() || path.extension() != AssetFileExtension)
				path += AssetFileExtension;
		}

		template<typename T>
		inline static bool TrySaveAsset(const std::filesystem::path& metadataPath, Asset<T> asset)
		{
			std::ofstream metaDataFile(metadataPath, std::ios::trunc);
			if (!metaDataFile.is_open())
				return false;

			JsonSerializer metaDataArchive(metaDataFile);
			// Add the guid
			metaDataArchive(CUSTOM_NAME(asset.GetAssetGuid(), "AssetGuid"));

			// template<typename Archive>
			// void SaveToAssetFile(Archive& metaDataArchive)
			if constexpr (HasSaveMetaOnly<T, JsonSerializer>)
			{
				(*asset.m_asset)->SaveToAssetFile(metaDataArchive);
			}
			else if constexpr (HasSaveMetaAndAsset<T, JsonSerializer>)
			{
				// Open the asset file
				bool bin = AssetTypes::GetAssetType<T>().binary;
				std::filesystem::path assetPath = metadataPath;
				assetPath.replace_extension(""); // Important : do the replace on a copy, other it will change in the registy
				std::ofstream assetFile(assetPath, bin ? std::ios::trunc | std::ios::binary : std::ios::trunc); // remove the .asset

				if (assetFile.is_open())
				{
					(*asset.m_asset)->SaveToAssetFile(metaDataArchive, assetFile);
				}
			}

			return true;
		}

		static void LoadRegistryRecursive(const std::filesystem::path& path);

		// Clear and delete all the assets
		// this should be called on EngineEvent::OnEngineStop
		inline static void Clear()
		{
			s_assets.clear();
			s_registry.clear();
		}

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStop().Register<&AssetManager::Clear>();
		});


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