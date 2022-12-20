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
#include "../events/EngineEvents.h"

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
			bool bin = AssetTypes::GetAssetType<T>().binary;
			std::ifstream assetFile(path->second.string().substr(0, path->second.string().length() - 6), bin ? std::ios::binary : std::ios::in); // Remove the .asset part, std::ios::in used as a dummy

			if (!metaDataFile.is_open() || !assetFile.is_open())
				return Asset<T>(); // Failed

			JsonDeserializer metaDataArchive(metaDataFile);
			a.SetAssetGuid(guid);
			a.m_asset = T::LoadFromAssetFile(guid, metaDataArchive, assetFile);

			s_assets[guid] = a;

			return a;
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

			// Load the asset from the file
			std::ofstream metaDataFile(path->second, std::ios::trunc);

			if (!metaDataFile.is_open())
				return false; // Failed to open the file

			auto asset = std::any_cast<Asset<T>>(assetAny->second);

			// template<typename Archive>
			// void SaveToAssetFile(Archive& metaDataArchive)
			if constexpr (HasSaveMetaOnly<T, JsonSerializer>)
			{
				JsonSerializer metaDataArchive(metaDataFile);
				asset.m_asset->SaveToAssetFile(metaDataArchive);
			}
			else if constexpr (HasSaveMetaAndAsset<T, JsonSerializer>)
			{
				// Open the asset file
				bool bin = AssetTypes::GetAssetType<T>().binary;
				std::filesystem::path assetPath = path->second;
				assetPath.replace_extension(""); // Important : do the replace on a copy, other it will change in the registy
				std::ofstream assetFile(assetPath, bin ? std::ios::trunc | std::ios::binary : std::ios::trunc); // remove the .asset
					
				if (assetFile.is_open())
				{
					JsonSerializer metaDataArchive(metaDataFile);
					asset.m_asset->SaveToAssetFile(metaDataArchive, assetFile);
				}
				else
				{
					metaDataFile << "{}"; // the metaDataFile has to be empty, add an empty json node
					return false;
				}
			}

			if (metaDataFile.cur == 1)
			{ // If the file is still empty, add a json node
				metaDataFile << "{}";
			}

			return true;
		}

		// Add the asset to the registry, returns false if the file could not be opened/created
		// assetPath can have the .asset extension or not
		// This will remove any existing assets with this guid
		template<typename T>
		inline static bool AddAsset(const Guid& guid, std::filesystem::path assetPath)
		{
			AddMetaExtension(assetPath);
			std::ofstream assetFile(assetPath, std::ios::trunc);
			assetFile << "{}"; // Empty json object (JsonSerializer)

			s_registry[guid] = assetPath;

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

		// add the .asset after the path
		inline static void AddMetaExtension(std::filesystem::path& path)
		{
			if (!path.has_extension() || path.extension() != Asset<int>::FileExtension)
				path += Asset<int>::FileExtension;
		}

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