#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "glaze/glaze.hpp"

namespace ASE {

	// Represents the "bounds" object within a cel.
	struct bounds_t {
		int y{};
		int x{};
		int width{};
		int height{};
	};

	struct Point {
		double x{};
		double y{};

		struct glaze {
			using T = Point;
			static constexpr auto value = glz::object( "x", &T::x, "y", &T::y );
		};
	};

	struct cell_user_data_physics_t {
		std::optional<std::vector<Point>> polygonCollisionShape{};

		struct glaze {
			using T = cell_user_data_physics_t;
			static constexpr auto value = glz::object( "polygonCollisionShape", &T::polygonCollisionShape );
		};

	};

	// Represents a "cel" object within a layer.
	// std::optional is used for fields that may not be present in every cel.
	struct cel_t {
		bounds_t bounds{};
		int frame{};
		std::optional<std::string> image{};
		std::optional<std::string> data{};
	};

	// Represents a "layer" object.
	struct layer_t {
		std::string name{};
		std::vector<cel_t> cells{};
	};

	// Represents a "tag" object for animations.
	struct tag_t {
		std::string color{};
		int to{};
		std::string aniDir{};
		int from{};
		std::string name{};
	};

	// Represents a "frame" object with its duration.
	struct frame_t {
		double duration{};
	};

	// The top-level structure for the entire Aseprite JSON data.
	struct aseprite_file_t {
		// The JSON shows an empty tilesets array. If it contained data,
		// a corresponding struct would be needed here. For now, we can
		// represent it as a vector of an empty struct or a generic JSON object.
		std::vector<glz::json_t> tilesets{};
		int width{};
		std::vector<tag_t> tags{};
		std::vector<frame_t> frames{};
		std::string filename{};
		int height{};
		std::vector<layer_t> layers{};
	};
}

//
//// Example of how to use the structure with Glaze
//int main( ) {
//	// The JSON data provided in the user request.
//	std::string json_data = R"({
//        "tilesets": [],
//        "width": 200,
//        "tags": [{"color": "#000000", "to": 12, "aniDir": "forward", "from": 2, "name": "Loop"}],
//        "frames": [
//            {"duration": 0.1}, {"duration": 0.1}, {"duration": 0.1},
//            {"duration": 0.1}, {"duration": 0.1}, {"duration": 0.1},
//            {"duration": 0.1}, {"duration": 0.1}, {"duration": 0.1},
//            {"duration": 0.1}, {"duration": 0.1}, {"duration": 0.1},
//            {"duration": 0.1}
//        ],
//        "filename": "D:\\code\\SiegePerilous\\content\\base\\sprites\\character\\nobg_0001.ase",
//        "height": 400,
//        "layers": [
//            {
//                "name": "Layer",
//                "cels": [
//                    {"bounds": {"y": 13, "x": 38, "width": 130, "height": 381}, "frame": 0, "image": "D:\\...\\image1.png"},
//                    {"bounds": {"y": 19, "x": 25, "width": 137, "height": 380}, "frame": 1, "image": "D:\\...\\image2.png"},
//                    {"data": "...", "bounds": {"y": 24, "x": 38, "width": 124, "height": 376}, "frame": 2, "image": "D:\\...\\image3.png"}
//                ]
//            }
//        ]
//    })";
//
//	aseprite_file_t aseprite_data{};
//	// The glz::read_json function parses the JSON string into the C++ struct.
//	auto error = glz::read_json( aseprite_data, json_data );
//
//	if ( error ) {
//		std::cerr << "Failed to parse JSON: " << glz::format_error( error, json_data ) << std::endl;
//		return 1;
//	}
//
//	// You can now access the parsed data using the C++ objects.
//	std::cout << "Successfully parsed file: " << aseprite_data.filename << std::endl;
//	std::cout << "Image dimensions: " << aseprite_data.width << "x" << aseprite_data.height << std::endl;
//	std::cout << "Number of layers: " << aseprite_data.layers.size( ) << std::endl;
//	if ( !aseprite_data.layers.empty( ) ) {
//		std::cout << "First layer name: " << aseprite_data.layers[0].name << std::endl;
//		std::cout << "Number of cels in first layer: " << aseprite_data.layers[0].cels.size( ) << std::endl;
//	}
//	if ( !aseprite_data.tags.empty( ) ) {
//		std::cout << "First tag name: " << aseprite_data.tags[0].name << std::endl;
//	}
//
//	return 0;
//}