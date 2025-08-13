//
//
//
//   SPACE SAVER TILED TOOLS
//
//
//
//
//
//
//

/*
 * Custom Exporter for Tiled Map Editor
 *
 * This script adds a "Export To Folder" action to the File menu.
 *
 * When triggered, it will:
 * 1. Ask for an gameconfig ifnot already set.
 * 2. Copy all images from tilesets to a subdirectory called "images" or "sprites" if they are aseprite files.
 * 2a. If aseprite files are found, it will run a custom exporter with the aseprite commandline that exports the sprite images to png files, and the sprite it self to /sprites/[name]/sprite.json.
 * 3. Change all paths to be relative to the base location of the game config
 * 4. Export all map as a JSON file to the selected directory.
 */

const PROP_GameConfig = 'GameConfig';
const PROP_TileCollectionFolder = 'Folder';
const PROP_AseExeLocation = 'AseSpritePath';

var customExporter = tiled.registerAction("SiegePerilousExport", function (action) {
	var map = tiled.activeAsset;

	if (!map || !map.isTileMap) {
		tiled.alert("Please open a map before running this script.");
		return;
	}

	var gameConfigFile = tiled.activeAsset.resolvedProperty(PROP_GameConfig);
	if (!gameConfigFile) {
		gameConfigFile = tiled.promptOpenFile("D:/code/SiegePerilous/config.json", "Game Config (*.json)");
		map.setProperty(PROP_GameConfig, gameConfigFile);
	}

	var aseSpritePath = tiled.activeAsset.resolvedProperty(PROP_AseExeLocation);

	var file = new TextFile(gameConfigFile, TextFile.ReadOnly);
	var file_content = file.readAll();
	var config = JSON.parse(file_content);
	config.basePath += "/base"
	tiled.log("Base Path: " + config.basePath);
	tiled.log("Save Path: " + config.savePath);

	var mapFileName = FileInfo.fileName(map.fileName).replace(/\.[^/.]+$/, "");
	var exportPath = config.basePath + "/tileMaps"
	var exportJsonPath = exportPath + "/" + mapFileName + ".json";
	var tmpJsonPath = exportPath + "/tmp_" + mapFileName + ".json";
	var imageExportDir = config.basePath + "/images";
	var tileSetExportDir = config.basePath + "/tileSets";
	var spriteExportDir = config.basePath + "/sprites";

	if (!File.exists(exportPath)) {
		File.makePath(exportPath);
	}
	if (!File.exists(tileSetExportDir)) {
		File.makePath(tileSetExportDir);
	}
	if (!File.exists(imageExportDir)) {
		File.makePath(imageExportDir);
	}
	if (!File.exists(spriteExportDir)) {
		File.makePath(spriteExportDir);
	}

	let mapData = null;
	let jsonMapFormat = tiled.mapFormat("json");
	let jsonTileFormat = tiled.tilesetFormat("json");
	try {
		//copy and modify all tilesets
		for (var i = 0; i < map.tilesets.length; ++i) {
			var tmpTileSet;
			var tileset = map.tilesets[i];
			var tmpTileSetPath = tileSetExportDir + "/" + FileInfo.fileName(tileset.fileName);

			tiled.log("copying tileset to " + tmpTileSetPath)

			jsonTileFormat.write(tileset, tmpTileSetPath)

			var tileSetfile = new TextFile(tmpTileSetPath, TextFile.ReadOnly);
			var tileSetfile_content = tileSetfile.readAll();
			var TileSetfileJs = JSON.parse(tileSetfile_content);
			tileSetfile.close();

			//tiled.log(" > " + JSON.stringify(TileSetfileJs, null, 1));

			let tileCollectionFolder;

			if (!TileSetfileJs.properties) {
				TileSetfileJs.properties = [];
			}

			TileSetfileJs.properties.forEach(
				prop => { if (prop.name == "Folder") tileCollectionFolder = prop.value; }
			);

			if (!tileCollectionFolder && !TileSetfileJs.image) {
				tileCollectionFolder = tiled.promptDirectory("D:/code/SiegePerilous/", "Select a Folder for image collection");
				TileSetfileJs.setProperty(PROP_TileCollectionFolder, tileCollectionFolder);
			}

			var newFolderPath = imageExportDir;

			if (tileCollectionFolder)
				newFolderPath += "/" + tileCollectionFolder;

			if (!File.exists(newFolderPath)) {
				tiled.warn("Folder does not exist <o> " + newFolderPath);
				File.makePath(newFolderPath)
				tiled.warn("Folder <o> " + newFolderPath + " created! ");
			}

			if (TileSetfileJs.image) {

				var imageFileName = FileInfo.fileName(TileSetfileJs.image)
				var absoluteImagePath = FileInfo.cleanPath(TileSetfileJs.image);
				var newImagePath = newFolderPath;
				newImagePath += "/" + imageFileName;

				// Copy the image
				if (File.exists(absoluteImagePath)) {
					File.copy(absoluteImagePath, newImagePath);
				} else {
					tiled.alert("Could not find image: " + absoluteImagePath);
				}
				TileSetfileJs.image = "images/" + tileCollectionFolder + "/";
				TileSetfileJs.image += imageFileName;
			}
			else {
				TileSetfileJs.tiles.forEach(tile => {

					// Access the image path for each tile
					imagePath = tile.image;

					tiled.log(" >> Processing image: " + imagePath);

					if (!imagePath)
						return;

					var imageFileName = FileInfo.fileName(imagePath)
					var absoluteImagePath = FileInfo.cleanPath(imagePath);
					var newImagePath = newFolderPath + "/" + imageFileName;



					// if ase, run ase json commandline Exporter
					if (FileInfo.completeSuffix(imagePath) == "aseprite"
						|| FileInfo.completeSuffix(imagePath) == "ase") {
						newFolderPath = spriteExportDir;


						if (tileCollectionFolder)
							newFolderPath += "/" + tileCollectionFolder;

						newImagePath = newFolderPath + "/" + imageFileName;

						if (!File.exists(newFolderPath)) {
							tiled.warn("Folder does not exist <o> " + newFolderPath);
							File.makePath(newFolderPath)
							tiled.warn("Folder <o> " + newFolderPath + " created! ");
						}

						// Copy the sprite
						if (File.exists(absoluteImagePath)) {
							File.copy(absoluteImagePath, newImagePath);
						} else {
							tiled.alert("Could not find sprite: " + absoluteImagePath);
						}

						var aseSpriteLocation = tiled.activeAsset.resolvedProperty(PROP_AseExeLocation);
						if (!aseSpriteLocation) {
							aseSpriteLocation = tiled.promptOpenFile("D:/games/steam/steamapps/common/Aseprite/Asesprite.exe", "Asesprite location (*.exe)");
							map.setProperty(PROP_AseExeLocation, aseSpriteLocation);
						}

						if (!File.exists(aseSpriteLocation)) {
							tiled.error(" invalid aseSpriteLocation ! \n Set '" + PROP_AseExeLocation + "' to your full AseSprite.exe path as custom prop on map");
							return;
						}
						tiled.log(" >> Starting ase export in  " + newFolderPath + "/");

						var args = ["-b", newImagePath, "-script", "C:/Users/06162/AppData/Roaming/Aseprite/scripts/diffme.lua"];
						//["-b",newImagePath,"-script","C:/Users/06162/AppData/Roaming/Aseprite/scripts/export.lua"]


						var proc = new Process();
						tiled.log(aseSpriteLocation + " " + args);
						proc.workingDirectory = newFolderPath + "/";
						proc.exec(aseSpriteLocation, args, false);

						File.remove(newImagePath)
						tile.image = "sprites/" + tileCollectionFolder + "/" + imageFileName;

					}
					else {
						// Copy the image
						if (File.exists(absoluteImagePath)) {
							File.copy(absoluteImagePath, newImagePath);
						} else {
							tiled.alert("Could not find image: " + absoluteImagePath);
						}

						tile.image = "images/" + tileCollectionFolder + "/" + imageFileName;
					}


				});
			}

			const updatedJsonString = JSON.stringify(TileSetfileJs, null, 2);

			// Create a TextFile object for writing
			const outputFile = new TextFile(tmpTileSetPath, TextFile.WriteOnly);

			try {
				outputFile.write(updatedJsonString);
				outputFile.commit(); // This saves the changes to the disk
				tiled.log("Successfully saved changes to: " + tmpTileSetPath);
			} catch (e) {
				tiled.error("Failed to write to file: " + e);
			}
		}

		// Write the complete map data to the temporary file.
		tiled.log("copying tileMap to " + exportJsonPath)
		jsonMapFormat.write(map, exportJsonPath);

		var tilemapfile = new TextFile(exportJsonPath, TextFile.ReadOnly);
		var tilemapfile_content = tilemapfile.readAll();
		var tilemapfileJs = JSON.parse(tilemapfile_content);
		tilemapfile.close();

		tilemapfileJs.tilesets.forEach(tileset => {
			tileset.source = "tileSets/" + FileInfo.fileName(tileset.source);
			tiled.log("modifying tileset source to " + tileset.source)
		});

		const updatedJsonString = JSON.stringify(tilemapfileJs, null, 2);

		// Create a TextFile object for writing
		const outputFile = new TextFile(exportJsonPath, TextFile.WriteOnly);

		try {
			outputFile.write(updatedJsonString);
			outputFile.commit(); // This saves the changes to the disk
			tiled.log("Successfully saved changes to: " + exportJsonPath);
		} catch (e) {
			tiled.error("Failed to write to file: " + e);
		}

	} finally {
		//tiled.log("Map exported to: " + exportJsonPath);
	}

	tiled.alert("Map exported to: " + exportJsonPath);
});

customExporter.text = "Export To Folder";

tiled.extendMenu("Map", [
	{ separator: true },
	{ action: "SiegePerilousExport" }
]);


const TILED_VERSION = tiled.version.split('.').map((e, i) => e * 100 ** (2 - i)).reduce((a, b) => a + b);

const MAP_WIDTH = 128;
const MAP_HEIGHT = 64;


function ss_write(tm, filename) {
	tiled.log(`StarSaver ss_write`);

	var jsonFormat = tiled.mapFormat("json");
	if (!jsonFormat) {
		tiled.alert("JSON map format not found.");
		return;
	}
	jsonFormat.write(tm, filename);

	return tm
}

if (TILED_VERSION >= 10500) {
	const starsaver_format =
	{
		name: 'StarSaver (*.json)',
		extension: 'json',
		write: ss_write,
	};

	//tiled.registerMapFormat('StarSaver', starsaver_format);
	tiled.log(`StarSaver tools registred`);
}
else {

	tiled.warn(`Tiled version ${tiled.version} is too old for the STARSAVER plugin (1.5.0 required)`);

}