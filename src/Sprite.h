#pragma once
#include "FileSystem.h"

namespace SiegePerilous {

	class AseSprite {
	public:
		// The required static data member.
		static const std::string extension;
		std::string data;

		// The required non-static 'Read' function.
		bool Read( const std::filesystem::path &filename ) {

			//dont have to check if the file exists, cachec takes care of that
			bool ok = true;
			if ( ok )
			{
				data = "Initialized from file";
				std::cout << "Instance method Read() successful for: " << filename << std::endl;
				return true;
			}
			std::cout << "Instance method Read() failed for: " << filename << std::endl;
			return false;
		}
	};

	const std::string AseSprite::extension = ".ase";

	void LoadSprite( const char *filename, int width, int height, int xOffset, int yOffset ) {};
}


