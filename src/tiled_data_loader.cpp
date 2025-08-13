#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3/SDL_assert.h>
#include "tiled_data.h"
#include <zlib.h>
#include "FileSystem.h"

namespace Tiled {

	bool decompress_zlib( const std::string &compressed_data, std::vector<unsigned char> &decompressed_data ) {
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = compressed_data.size( );
		stream.next_in = ( Bytef * ) compressed_data.data( );

		// Use inflateInit2 with a windowBits of 15+32 for automatic zlib/gzip header detection.
		if ( inflateInit2( &stream, 15 + 32 ) != Z_OK ) {
			return false;
		}

		int ret;
		const size_t CHUNK = 32768;
		unsigned char outbuffer[CHUNK];

		do {
			stream.avail_out = CHUNK;
			stream.next_out = ( Bytef * ) outbuffer;
			ret = inflate( &stream, Z_NO_FLUSH );

			switch ( ret ) {
			case Z_NEED_DICT:
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				inflateEnd( &stream );
				return false;
			}

			unsigned int have = CHUNK - stream.avail_out;
			decompressed_data.insert( decompressed_data.end( ), outbuffer, outbuffer + have );

		} while ( ret != Z_STREAM_END );

		inflateEnd( &stream );
		return ret == Z_STREAM_END;
	}

	// Loads a Tiled map and recursively resolves its external tilesets
	std::optional<Tiled::Map> load_map_with_deps( const std::string &map_path ) {
		// --- 1. Load the main map file ---
		size_t map_file_size = 0;		
		auto map_buffer_data = fileSystem->ReadFile( map_path );
		Tiled::Map map;
		if (auto& buffer = map_buffer_data) {
			// --- 2. Parse the main map buffer into our Tiled::Map struct ---
			std::cout << "Successfully loaded '" << map_path << "' (" << buffer->size() << " bytes)." << std::endl;
			auto err = glz::read < glz::opts{ .error_on_unknown_keys = false } > ( map, *buffer);
			if ( err ) {
				std::cerr << "Error: Failed to parse map JSON from '" << map_path << "'." << std::endl;
				return std::nullopt;
			}
		}
		else {
			std::cerr << "Error: Could not load map file '" << map_path << "'. SDL_Error: " << SDL_GetError( ) << std::endl;
			return std::nullopt;
		}

		std::cout << "Successfully parsed '" << map_path << "'." << std::endl;

		// --- 3. Resolve external tilesets ---
		for ( auto &tileset : map.tilesets ) {
			if ( tileset.source ) {
				std::string tileset_path = *tileset.source;
				std::cout << "> Found external tileset source: '" << *tileset.source << "'. Loading from '" << fileSystem->RelativeToOSPath( tileset_path ) << "'" << std::endl;

				size_t tileset_file_size = 0;
				
				auto tileset_buffer_data = fileSystem->ReadFile( tileset_path );

				if ( auto &buffer = tileset_buffer_data ) {
					// --- Parse the buffer into our struct ---
					std::cout << "  Successfully loaded '" << tileset_path << "' (" << buffer->size( ) << " bytes)." << std::endl;
					auto err = glz::read < glz::opts{ .error_on_unknown_keys = false } > ( tileset, *buffer );
					if ( err ) {
						std::cerr << "Error: Failed to parse map JSON from '" << map_path << "'." << std::endl;
						return std::nullopt;
					}
				} else {
					std::cerr << "Error: Could not load map file '" << map_path << "'. SDL_Error: " << SDL_GetError( ) << std::endl;
					return std::nullopt;
				}

				std::cout << "  Successfully parsed and merged tileset '" << ( tileset.name ? *tileset.name : "N/A" ) << "'." << std::endl;
			}
		}

		// --- 4. Decode layer data ---
		for ( auto &layerRefPtr : map.GetAllLayersOfType( "tilelayer" ,true) ) {
			Tiled::Layer &layer = *layerRefPtr;
			if ( layer.data.has_value( ) ) {
				//if the variant is a string, its Base64 encoded data?
				//when it are numbers it is already decoded?
				if ( std::holds_alternative<std::string>( layer.data.value( ) ) ) {
					const std::string &encoded_data = std::get<std::string>( layer.data.value( ) );

					SDL_assert( !encoded_data.empty( ) && layer.encoding == "base64" );

					// 1. Decode Base64 data
					std::string decoded_b64_str = glz::read_base64( encoded_data );
					if ( decoded_b64_str.empty( ) && !encoded_data.empty( ) ) {
						std::cerr << "Error: Base64 decoding failed for layer '" << layer.name << "'." << std::endl;
						continue;
					}

					std::vector<unsigned char> current_data( decoded_b64_str.begin( ), decoded_b64_str.end( ) );

					// 2. Decompress if necessary
					if ( layer.compression && *layer.compression == "zlib" ) {
						std::vector<unsigned char> decompressed_data;
						// Pass the decoded base64 string directly to the decompressor
						if ( decompress_zlib( decoded_b64_str, decompressed_data ) ) {
							current_data = std::move( decompressed_data );
						} else {
							std::cerr << "Error: Failed to decompress zlib data for layer '" << layer.name << "'." << std::endl;
							continue;
						}
					}

					// 3. Safely copy the final byte data into the uint32_t vector
					if ( current_data.size( ) % sizeof( uint32_t ) != 0 ) {
						std::cerr << "Error: Decompressed data size is not a multiple of 4 for layer '" << layer.name << "'." << std::endl;
						continue;
					}

					// Reinterpret the data without using memcpy
					layer.decoded_data.resize( current_data.size( ) / sizeof( uint32_t ) );
					std::memcpy( layer.decoded_data.data( ), current_data.data( ), current_data.size( ) );


				} else if ( std::holds_alternative<std::vector<uint32_t>>( layer.data.value( ) ) ) {
					layer.decoded_data = std::get<std::vector<uint32_t>>( layer.data.value( ) );
				}
			}
		}

		return map;
	}

	namespace { // Use an anonymous namespace to keep the helper function private to this file.

		// Recursively searches through a vector of layers to find all layers of a specific type.
		void findLayersRecursive(
			std::vector<Layer> &layersToSearch,      // The current list of layers to iterate through.
			const std::string &type,                  // The layer type we are looking for.
			bool isParentVisible,                     // The visibility status of the parent group.
			const bool targetVisibility,              // The desired visibility state (true for visible, false for hidden).
			const bool shouldResolveGroups,           // Whether to search inside group layers.
			std::vector<Layer *> &result )              // The vector to store pointers to found layers.
		{
			for ( auto &layer : layersToSearch ) {
				// A layer is effectively visible only if its own 'visible' flag is true AND its parent is visible.
				const bool currentLayerIsEffectivelyVisible = isParentVisible && layer.visible;

				// If the layer's type matches what we're looking for...
				if ( layer.type == type ) {
					// ...and its effective visibility matches the target visibility, add it to the results.
					if ( currentLayerIsEffectivelyVisible == targetVisibility ) {
						result.push_back( &layer );
					}
				}

				// If we are supposed to resolve groups, this layer is a group, and it has sub-layers, then recurse.
				if ( shouldResolveGroups && layer.type == "group" && layer.layers.has_value( ) ) {
					// The 'isParentVisible' for the next level of recursion is this layer's effective visibility.
					findLayersRecursive(
						*layer.layers,
						type,
						currentLayerIsEffectivelyVisible,
						targetVisibility,
						shouldResolveGroups,
						result
					);
				}
			}
		}
	}

	// Returns all layers of a given type, taking into account visibility and group hierarchy.
	// This is implemented as a member function of Tiled::Map.
	std::vector<Layer *> Map::GetLayersOfType( const std::string &type, const bool visible, const bool resolveGroup ) {
		std::vector<Layer *> foundLayers;

		// Start the recursive search from the map's top-level layers.
		// The initial "parent" (the map itself) is considered visible, so we pass 'true'.
		findLayersRecursive( this->layers, type, true, visible, resolveGroup, foundLayers );

		return foundLayers;
	}

	// Provides all layers of a specific type, ignoring their visibility.
	// It merges the visible and invisible layers into a single vector.
	std::vector<Layer *> Tiled::Map::GetAllLayersOfType( const std::string &type, bool resolveGroup ) {
		// First, get all the visible layers of the specified type.
		std::vector<Layer *> allLayers = GetLayersOfType( type, true, resolveGroup );

		// Then, get all the invisible layers of the same type.
		std::vector<Layer *> invisibleLayers = GetLayersOfType( type, false, resolveGroup );

		// Append the invisible layers to the end of the vector containing the visible layers.
		allLayers.insert( allLayers.end( ), invisibleLayers.begin( ), invisibleLayers.end( ) );

		return allLayers;
	}





}