#pragma once

#include <string>
#include <memory>
#include <map>
#include <glaze/glaze.hpp>
#include "include/Declarations.h"
#include <typeindex>
#include <atomic>
#include <any>
#include "FileSystem.h"
#include <iostream>

namespace SiegePerilous
{
	using namespace System;

	namespace Content {


		namespace fs = std::filesystem;
		/**
		* @brief Removes the first directory component from a path.
		* 
		* This function correctly handles relative, absolute, and root-name paths.
		* For example:
		* - "a/b/c"      -> "b/c"
		* - "/a/b/c"     -> "b/c"
		* - "C:/a/b/c"   -> "b/c"
		* - "a"          -> ""
		* 
		* @param p The input path.
		* @return A new path with the first directory component removed.
		*/
		fs::path snip_first_folder(fs::path& p);

		template <typename T>
		bool contains( const std::vector<T> &vec, const T &value ) {
			return std::find( vec.begin( ), vec.end( ), value ) != vec.end( );
		}

		extern std::map<std::type_index, std::any>					content_caches;
		extern std::map<std::type_index, std::filesystem::path>		content_caches_path;
		extern std::map<std::type_index, std::atomic<bool>>			content_caches_dirty;

		template<class T>
		class content_cache_item {
		public:
			content_cache_item( std::filesystem::path path ) {
				if ( content_caches_path.find( typeid( T ) ) ==content_caches_path.end( ) ) {
					content_caches_path[typeid( T )] = path; // Initialize the path for this type
				} else {
					std::cerr << "Warning: Path for type " << typeid( T ).name( ) << " already exists. Using existing path." << std::endl;
				}
			}
		};

		// Helper to get/create the dirty flag for a given type
		template<typename T>
		std::atomic<bool> &get_dirty_flag( ) {
			std::type_index type_idx = typeid( T );
			if ( content_caches_dirty.find( type_idx ) == content_caches_dirty.end( ) ) {
				content_caches_dirty[type_idx] = true;
			}
			return content_caches_dirty[type_idx];
		}

		// Helper to get/create the cache map for a given type
		template<typename T>
		std::map<std::string, T> &get_cache_map( ) {
			std::type_index type_idx = typeid( T );
			if ( content_caches.find( type_idx ) == content_caches.end( ) ) {
				content_caches[type_idx] = std::map<std::string, T>{};
				get_dirty_flag<T>( ) = true;
			}
			try {
				return std::any_cast< std::map<std::string, T> & >( content_caches[type_idx] );
			}
			catch ( const std::bad_any_cast &e ) {
				std::cerr << "FATAL ERROR: Bad any_cast for type " << typeid( T ).name( ) << " cache: " << e.what( ) << std::endl;
				throw std::runtime_error( "Internal cache error: Bad type cast." );
			}
		}

		template<typename T>
		void store_in_cache( const std::string &id, T &obj ) {
			std::type_index type_idx = typeid( T );

			// Get or create the inner map for this type
			if ( content_caches.find( type_idx ) == content_caches.end( ) ) {
				// Create a new map for this type and store it in the any
				content_caches[type_idx] = std::map<std::string, T>{};
				content_caches_dirty[type_idx] = true; // New cache is initially dirty (needs loading)
			}

			try {
				auto &inner_map = std::any_cast< std::map<std::string, T> & >( content_caches[type_idx] );
				inner_map[id] = obj;
				content_caches_dirty[type_idx] = true;
			}
			catch ( const std::bad_any_cast &e ) {
				std::cerr << "Error: Bad any_cast when storing type " << typeid( T ).name( ) << ": " << e.what( ) << std::endl;
			}
		}

		template<typename T>
		std::optional<T> get_from_cache( const std::string &id ) {
			std::type_index type_idx = typeid( T );

			auto cache_it = content_caches.find( type_idx );
			if ( cache_it == content_caches.end( ) ) {
				std::cerr << "Warning: No path set on cache for type " << typeid( T ).name( ) << std::endl;
				return std::nullopt; // No cache for this type
			}

			// Retrieve the inner map (requires casting)
			try {
				auto &inner_map = std::any_cast< std::map<std::string, T> & >( cache_it->second );
				auto obj_it = inner_map.find( id );
				if ( obj_it != inner_map.end( ) ) {
					return obj_it->second; // Found in cache
				} else {
					return std::nullopt; // Not found in inner map
				}
			}
			catch ( const std::bad_any_cast &e ) {
				std::cerr << "Error: Bad any_cast when retrieving type " << typeid( T ).name( ) << ": " << e.what( ) << std::endl;
				return std::nullopt; // Indicate failure
			}
		}

		template<typename T>
		concept CanBePopulatedFromFile = requires( T obj, const std::filesystem::path & p ) {
			// Requirement for default construction
			requires std::default_initializable<T>;

			// Requirement for the non-static update function
			{ obj.Read( p ) } -> std::same_as<bool>;

			// Requirement for a static 'extension' data member
			{ T::extension } -> std::convertible_to<const std::string &>;
		};

		template<CanBePopulatedFromFile T>
		std::optional<T> Load( const std::filesystem::path &filename ) {
			
			if ( auto cached_content = get_from_cache<T>( filename.string( ) ) ) {
				// If it does, we return it immediately. The function stops here.
				return cached_content;
			}
			
			auto content_type_path = content_caches_path[typeid( T )];

			if ( content_type_path.empty( ) ) {
				std::cerr << "Warning: No path set for type " << typeid( T ).name( ) << std::endl;
				return std::nullopt;
			}
			
			std::filesystem::path localPath = filename;
			auto firstPath = snip_first_folder( localPath );

			if ( content_type_path != firstPath ) {
				std::cerr << "Warning: No the correct type " << typeid( T ).name( ) << " should be " << content_type_path << " not " << firstPath << std::endl;
				return std::nullopt;
			}

			///std::string ext = T::extension;
			//filename.replace_extension(ext);
			const std::filesystem::path file_path = content_type_path / filename ;

			if ( ! fileSystem->Exists( filename ) ) {
				return std::nullopt; // Not found
			}

			T content; 
			if ( content.Read( filename ) ) {
				store_in_cache<T>( filename.string( ), content ); // Store the empty object in cache
				return content;
			}

			return std::nullopt;
		}


	}

	//class ContentItem {
	//public:
	//	virtual ~ContentItem( ) = default;
	//	virtual void Load( glz::json_t &json ) = 0;
	//};
	//
    //class ContentFactory
    //{
    //public:
    //    void RegisterItem(const std::string& type, std::unique_ptr<ContentItem> creator);
    //    const ContentItem* Get(const std::string& type) const;
	//
    //private:
    //    std::map<std::string, std::unique_ptr<ContentItem>> m_items;
    //};
	//
	//
	//// PhysicsShapeCreator for ASE sprites
	//class TileContent : public ContentItem {
	//public:
	//	//void load( glz::json_t &json ) const override;
	//	UInt8 m_tileType = 0;
	//};

}
