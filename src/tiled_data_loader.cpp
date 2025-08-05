#include <iostream>
#include <string>
#include <filesystem> // For path manipulation
#include <SDL3/SDL.h>
#include "tiled_data.h"


namespace Tiled {

// Helper function to get the directory part of a file path
std::string get_base_path( const std::string &file_path ) {
	try {
		std::filesystem::path path( file_path );
		return path.parent_path( ).string( ) + "/";
	}
	catch ( ... ) {
		return "";
	}
}

// Loads a Tiled map and recursively resolves its external tilesets
std::optional<Tiled::Map> load_map_with_deps( const std::string &map_path ) {
	// --- 1. Load the main map file using SDL_LoadFile ---
	size_t map_file_size = 0;
	char *map_buffer = ( char * ) SDL_LoadFile( map_path.c_str( ), &map_file_size );

	if ( !map_buffer ) {
		std::cerr << "Error: Could not load map file '" << map_path << "'. SDL_Error: " << SDL_GetError( ) << std::endl;
		return std::nullopt;
	}

	std::cout << "Successfully loaded '" << map_path << "' (" << map_file_size << " bytes)." << std::endl;

	// --- 2. Parse the main map buffer into our Tiled::Map struct ---
	Tiled::Map map;
	auto err = glz::read_json( map, std::string_view( map_buffer, map_file_size ) );
	SDL_free( map_buffer ); // Free the buffer now that we're done with it

	if ( err ) {
		std::cerr << "Error: Failed to parse map JSON from '" << map_path << "'." << std::endl;
		// Optional: print glaze error details
		// std::cerr << glz::format_error(err, map_buffer) << std::endl;
		return std::nullopt;
	}

	std::cout << "Successfully parsed '" << map_path << "'." << std::endl;

	// --- 3. Resolve external tilesets ---
	std::string base_path = get_base_path( map_path );
	std::cout << "Resolving tilesets from base path: '" << base_path << "'" << std::endl;

	for ( auto &tileset : map.tilesets ) {
		if ( tileset.source ) {
			std::string tileset_path = base_path + *tileset.source;
			std::cout << "--> Found external tileset source: '" << *tileset.source << "'. Loading from '" << tileset_path << "'" << std::endl;

			size_t tileset_file_size = 0;
			char *tileset_buffer = ( char * ) SDL_LoadFile( tileset_path.c_str( ), &tileset_file_size );

			if ( !tileset_buffer ) {
				std::cerr << "Error: Could not load tileset file '" << tileset_path << "'. SDL_Error: " << SDL_GetError( ) << std::endl;
				continue; // Skip this tileset but continue with others
			}

			auto ts_err = glz::read_json( tileset, std::string_view( tileset_buffer, tileset_file_size ) );
			SDL_free( tileset_buffer );

			if ( ts_err ) {
				std::cerr << "Error: Failed to parse tileset JSON from '" << tileset_path << "'." << std::endl;
			} else {
				std::cout << "    ... Successfully parsed and merged tileset '" << ( tileset.name ? *tileset.name : "N/A" ) << "'." << std::endl;
			}
		}
	}

	return map;
}
}
//int main( int argc, char *argv[] ) {
//	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
//		std::cerr << "SDL_Init Error: " << SDL_GetError( ) << std::endl;
//		return 1;
//	}
//
//	// Assume the JSON files are in a 'data' subdirectory relative to the executable
//	const std::string map_file = "main_menu.json";
//
//	auto map_opt = load_map_with_deps( map_file );
//
//	if ( map_opt ) {
//		std::cout << "\n--- Load and Resolution Successful! ---" << std::endl;
//		Tiled::Map &map = *map_opt;
//		std::cout << "Map Class: " << ( map.class_property ? *map.class_property : "N/A" ) << std::endl;
//		std::cout << "Dimensions: " << map.width << "x" << map.height << " tiles" << std::endl;
//		std::cout << "Tile Size: " << map.tilewidth << "x" << map.tileheight << std::endl;
//		std::cout << "Number of layers: " << map.layers.size( ) << std::endl;
//		std::cout << "Number of tilesets: " << map.tilesets.size( ) << std::endl;
//
//		for ( const auto &ts : map.tilesets ) {
//			std::cout << "\nTileset Details:" << std::endl;
//			std::cout << "  - First GID: " << ts.firstgid << std::endl;
//			std::cout << "  - Source: " << ( ts.source ? *ts.source : "Embedded" ) << std::endl;
//			if ( ts.source ) { // These fields only exist for resolved tilesets
//				std::cout << "  - Name: " << ( ts.name ? *ts.name : "N/A" ) << std::endl;
//				std::cout << "  - Image: " << ( ts.image ? *ts.image : "N/A" ) << std::endl;
//				std::cout << "  - Tile Count: " << ( ts.tilecount ? *ts.tilecount : 0 ) << std::endl;
//			}
//		}
//	} else {
//		std::cerr << "\n--- Failed to load map '" << map_file << "' ---" << std::endl;
//	}
//
//	SDL_Quit( );
//	return 0;
//}