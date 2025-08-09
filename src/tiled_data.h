#pragma once

#include "glaze/glaze.hpp"
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <memory>

namespace Tiled {

	// Forward declarations for recursive structures
	struct Layer;

	struct Property {
		std::string name{};
		std::string type{};
		std::variant<std::string, int, double, bool> value{};
		std::optional<std::string> propertytype{};

		struct glaze {
			using T = Property;
			static constexpr auto value = glz::object(
				"name", &T::name,
				"type", &T::type,
				"value", &T::value,
				"propertytype", &T::propertytype
			);
		};
	};

	struct Point {
		double x{};
		double y{};

		struct glaze {
			using T = Point;
			static constexpr auto value = glz::object( "x", &T::x, "y", &T::y );
		};
	};

	struct Text {
		bool bold = false;
		std::optional<std::string> color{};
		std::string fontfamily = "sans-serif";
		std::string halign = "left";
		bool italic = false;
		bool kerning = true;
		int pixelsize = 16;
		bool strikeout = false;
		std::string text{};
		bool underline = false;
		std::string valign = "top";
		bool wrap = false;

		struct glaze {
			using T = Text;
			static constexpr auto value = glz::object( "bold", &T::bold, "color", &T::color, "fontfamily", &T::fontfamily, "halign", &T::halign, "italic", &T::italic, "kerning", &T::kerning, "pixelsize", &T::pixelsize, "strikeout", &T::strikeout, "text", &T::text, "underline", &T::underline, "valign", &T::valign, "wrap", &T::wrap );
		};
	};

	struct Chunk {
		std::variant<std::vector<uint32_t>, std::string> data{};
		int height{};
		int width{};
		int x{};
		int y{};

		struct glaze {
			using T = Chunk;
			static constexpr auto value = glz::object( "data", &T::data, "height", &T::height, "width", &T::width, "x", &T::x, "y", &T::y );
		};
	};

	struct Frame {
		int duration{};
		int tileid{};

		struct glaze {
			using T = Frame;
			static constexpr auto value = glz::object( "duration", &T::duration, "tileid", &T::tileid );
		};
	};

	struct TileOffset {
		int x{};
		int y{};

		struct glaze {
			using T = TileOffset;
			static constexpr auto value = glz::object( "x", &T::x, "y", &T::y );
		};
	};

	struct Transformations {
		bool hflip{};
		bool vflip{};
		bool rotate{};
		bool preferuntransformed{};

		struct glaze {
			using T = Transformations;
			static constexpr auto value = glz::object( "hflip", &T::hflip, "vflip", &T::vflip, "rotate", &T::rotate, "preferuntransformed", &T::preferuntransformed );
		};
	};

	struct Grid {
		int height{};
		std::string orientation = "orthogonal";
		int width{};

		struct glaze {
			using T = Grid;
			static constexpr auto value = glz::object( "height", &T::height, "orientation", &T::orientation, "width", &T::width );
		};
	};

	struct Terrain {
		std::string name{};
		std::vector<Property> properties{};
		int tile{};

		struct glaze {
			using T = Terrain;
			static constexpr auto value = glz::object( "name", &T::name, "properties", &T::properties, "tile", &T::tile );
		};
	};

	struct WangColor {
		std::optional<std::string> class_property{};
		std::string color{};
		std::string name{};
		double probability{};
		std::vector<Property> properties{};
		int tile{};

		struct glaze {
			using T = WangColor;
			static constexpr auto value = glz::object( "class", &T::class_property, "color", &T::color, "name", &T::name, "probability", &T::probability, "properties", &T::properties, "tile", &T::tile );
		};
	};

	struct WangTile {
		int tileid{};
		std::vector<uint32_t> wangid{};

		struct glaze {
			using T = WangTile;
			static constexpr auto value = glz::object( "tileid", &T::tileid, "wangid", &T::wangid );
		};
	};

	struct WangSet {
		std::optional<std::string> class_property{};
		std::vector<WangColor> colors{};
		std::string name{};
		std::vector<Property> properties{};
		int tile{};
		std::string type{};
		std::vector<WangTile> wangtiles{};

		struct glaze {
			using T = WangSet;
			static constexpr auto value = glz::object( "class", &T::class_property, "colors", &T::colors, "name", &T::name, "properties", &T::properties, "tile", &T::tile, "type", &T::type, "wangtiles", &T::wangtiles );
		};
	};

	struct Tile {
		std::optional<std::vector<Frame>> animation{};
		int id{};
		std::optional<std::string> image{};
		std::optional<int> imageheight{};
		std::optional<int> imagewidth{};
		std::optional<std::unique_ptr<Layer>> objectgroup{};
		std::optional<double> probability{};
		std::vector<Property> properties{};
		std::optional<std::vector<int>> terrain{};
		std::optional<std::string> type{};

		struct glaze {
			using T = Tile;
			static constexpr auto value = glz::object( "animation", &T::animation, "id", &T::id, "image", &T::image, "imageheight", &T::imageheight, "imagewidth", &T::imagewidth, "objectgroup", &T::objectgroup, "probability", &T::probability, "properties", &T::properties, "terrain", &T::terrain, "type", &T::type );
		};
	};

	struct Object {
		bool ellipse = false;
		std::optional<uint32_t> gid{};
		double height{};
		int id{};
		std::string name{};
		bool point = false;
		std::optional<std::vector<Point>> polygon{};
		std::optional<std::vector<Point>> polyline{};
		std::vector<Property> properties{};
		double rotation{};
		std::optional<std::string> template_file{};
		std::optional<Text> text{};
		std::string type{};
		bool visible{};
		double width{};
		double x{};
		double y{};

		struct glaze {
			using T = Object;
			static constexpr auto value = glz::object( "ellipse", &T::ellipse, "gid", &T::gid, "height", &T::height, "id", &T::id, "name", &T::name, "point", &T::point, "polygon", &T::polygon, "polyline", &T::polyline, "properties", &T::properties, "rotation", &T::rotation, "template", &T::template_file, "text", &T::text, "type", &T::type, "visible", &T::visible, "width", &T::width, "x", &T::x, "y", &T::y );
		};
	};

	struct Layer {
		std::optional<std::vector<Chunk>> chunks{};
		std::optional<std::string> class_property{};
		std::optional<std::string> compression{};
		std::optional<std::variant<std::vector<uint32_t>, std::string>> data{};
		std::string draworder = "topdown";
		std::optional<std::string> encoding{};
		std::optional<int> height{};
		int id{};
		std::optional<std::string> image{};
		std::optional<std::vector<Layer>> layers{};
		bool locked = false;
		std::string name{};
		std::optional<std::vector<Object>> objects{};
		double offsetx = 0;
		double offsety = 0;
		double opacity = 1.0;
		double parallaxx = 1.0;
		double parallaxy = 1.0;
		std::vector<Property> properties{};
		std::optional<bool> repeatx{};
		std::optional<bool> repeaty{};
		std::optional<int> startx{};
		std::optional<int> starty{};
		std::optional<std::string> tintcolor{};
		std::optional<std::string> transparentcolor{};
		std::string type{};
		bool visible{};
		std::optional<int> width{};
		int x{};
		int y{};

		// This field is not part of the JSON, it will be populated after loading
		std::vector<uint32_t> decoded_data{};


		struct glaze {
			using T = Layer;
			static constexpr auto value = glz::object( 
					"chunks", &T::chunks, 
					"class", &T::class_property,
					"compression", &T::compression,
					"data", &T::data,
					"draworder", &T::draworder,
					"encoding", &T::encoding,
					"height", &T::height,
					"id", &T::id,
					"image", &T::image,
					"layers", &T::layers,
					"locked", &T::locked,
					"name", &T::name,
					"objects", &T::objects,
					"offsetx", &T::offsetx, "offsety", &T::offsety,
					"opacity", &T::opacity,
					"parallaxx", &T::parallaxx, "parallaxy", &T::parallaxy,
					"properties", &T::properties,
					"repeatx", &T::repeatx,	"repeaty", &T::repeaty,
					"startx", &T::startx,"starty", &T::starty,
					"tintcolor", &T::tintcolor,
					"transparentcolor", &T::transparentcolor,
					"type", &T::type,
					"visible", &T::visible,
					"width", &T::width,
					"x", &T::x, "y", &T::y );
		};
	};


	struct  Export {
		std::string format{};
		std::string target{};

		struct glaze {
			using  T = Export;
			static constexpr auto value = glz::object(
				"format", &T::format,
				"target", &T::target
			);
		};
	};

	struct EditorSettings {
		std::optional<Export> export_settings{};

		struct glaze {
			using T = EditorSettings;
			static constexpr auto value = glz::object(
				"export", &T::export_settings
			);
		};
	};

	struct Tileset {
		// Fields populated from the map file
		int firstgid{};
		std::optional<std::string> source{};
		std::optional<EditorSettings> editorsettings{};

		// Fields populated from the external tileset file
		std::optional<std::string> backgroundcolor{};
		std::optional<std::string> class_property{};
		std::optional<int> columns{};
		std::string fillmode = "stretch";
		std::optional<Grid> grid{};
		std::optional<std::string> image{};
		std::optional<int> imageheight{};
		std::optional<int> imagewidth{};
		std::optional<int> margin{};
		std::optional<std::string> name{};
		std::string objectalignment = "unspecified";
		std::vector<Property> properties{};
		std::optional<int> spacing{};
		std::optional<std::vector<Terrain>> terrains{};
		std::optional<int> tilecount{};
		std::optional<std::string> tiledversion{};
		std::optional<int> tileheight{};
		std::optional<TileOffset> tileoffset{};
		std::string tilerendersize = "tile";
		std::optional<std::vector<Tile>> tiles{};
		std::optional<int> tilewidth{};
		std::optional<Transformations> transformations{};
		std::optional<std::string> transparentcolor{};
		std::string type = "tileset";
		std::optional<std::string> version{};
		std::optional<std::vector<WangSet>> wangsets{};

		struct glaze {
			using T = Tileset;
			static constexpr auto value = glz::object(
				// Key from map file
				"firstgid", &T::firstgid,
				"source", &T::source,
				"editorsettings", &T::editorsettings,

				// Keys from external tileset file
				"backgroundcolor", &T::backgroundcolor,
				"class", &T::class_property,
				"columns", &T::columns,
				"fillmode", &T::fillmode,
				"grid", &T::grid,
				"image", &T::image,
				"imageheight", &T::imageheight,
				"imagewidth", &T::imagewidth,
				"margin", &T::margin,
				"name", &T::name,
				"objectalignment", &T::objectalignment,
				"properties", &T::properties,
				"spacing", &T::spacing,
				"terrains", &T::terrains,
				"tilecount", &T::tilecount,
				"tiledversion", &T::tiledversion,
				"tileheight", &T::tileheight,
				"tileoffset", &T::tileoffset,
				"tilerendersize", &T::tilerendersize,
				"tiles", &T::tiles,
				"tilewidth", &T::tilewidth,
				"transformations", &T::transformations,
				"transparentcolor", &T::transparentcolor,
				"type", &T::type,
				"version", &T::version,
				"wangsets", &T::wangsets
			);
		};
	};


	// Holds the static definition of an animation, loaded from the Tiled data.
	struct AnimationDefinition {
		// The GID of the tile that defines this animation (the first frame).
		uint32_t base_gid;
		// A vector of the frames that make up this animation.
		std::vector<Tiled::Frame> frames;
	};

	// Holds the current state of a running animation.
	struct ActiveAnimationState {
		// Pointer to the static animation data.
		const AnimationDefinition *definition;
		// The current frame index in the animation sequence.
		int current_frame_index = 0;
		// Accumulates time to determine when to switch frames.
		double time_accumulator_ms = 0.0;
	};


	struct Map {
		std::optional<std::string> backgroundcolor{};
		std::optional<std::string> class_property{};
		int compressionlevel = -1;
		int height{};
		std::optional<int> hexsidelength{};
		bool infinite{};
		std::vector<Layer> layers{};
		int nextlayerid{};
		int nextobjectid{};
		std::string orientation{};
		double parallaxoriginx = 0;
		double parallaxoriginy = 0;
		std::vector<Property> properties{};
		std::string renderorder = "right-down";
		std::optional<std::string> staggeraxis{};
		std::optional<std::string> staggerindex{};
		std::string tiledversion{};
		int tileheight{};
		std::vector<Tileset> tilesets{};
		int tilewidth{};
		std::string type = "map";
		std::string version{};
		int width{};
		//////////////////////////////////////////////////////////////////////////
		// runtime extras
		// Maps a base GID to its animation definition.
		std::map<uint32_t, AnimationDefinition> m_animation_definitions;
		// Maps a base GID to its currently active state.
		std::map<uint32_t, ActiveAnimationState> m_active_animations;
		//////////////////////////////////////////////////////////////////////////

		struct glaze {
			using T = Map;
			static constexpr auto value = glz::object(
				"backgroundcolor", &T::backgroundcolor,
				"class", &T::class_property,
				"compressionlevel", &T::compressionlevel,
				"height", &T::height,
				"hexsidelength", &T::hexsidelength,
				"infinite", &T::infinite,
				"layers", &T::layers,
				"nextlayerid", &T::nextlayerid,
				"nextobjectid", &T::nextobjectid,
				"orientation", &T::orientation,
				"parallaxoriginx", &T::parallaxoriginx,
				"parallaxoriginy", &T::parallaxoriginy,
				"properties", &T::properties,
				"renderorder", &T::renderorder,
				"staggeraxis", &T::staggeraxis,
				"staggerindex", &T::staggerindex,
				"tiledversion", &T::tiledversion,
				"tileheight", &T::tileheight,
				"tilesets", &T::tilesets,
				"tilewidth", &T::tilewidth,
				"type", &T::type,
				"version", &T::version,
				"width", &T::width
			);
		};

		std::vector<Layer *> GetAllLayersOfType( const std::string &type, bool resolveGroup );
		std::vector<Layer *> GetLayersOfType( const std::string &type, const bool visible, const bool resolveGroup );
	};
	
	// Loads a Tiled map and recursively resolves its external tilesets.
	std::optional<Tiled::Map> load_map_with_deps( const std::string &map_path );


} // namespace Tiled