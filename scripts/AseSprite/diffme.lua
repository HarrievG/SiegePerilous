-- export.lua
-- Copyright (C) 2020  David Capello
--
-- This file is released under the terms of the MIT license.
 
local spr = app.sprite
if not spr then spr = app.activeSprite end -- just to support older versions of Aseprite
if not spr then return print "No active sprite" end

local logFilename = 'D:/code/SiegePerilous/content/base/images/character/log.txt'
local logFile = io.open(logFilename, "a")
logFile:write("Starting export")


if ColorMode.TILEMAP == nil then ColorMode.TILEMAP = 4 end
assert(ColorMode.TILEMAP == 4)
logFile:write("\nColorMode.TILEMAP asserted to be 4.")

local fs = app.fs
local pc = app.pixelColor
local output_folder = fs.joinPath(app.fs.filePath(spr.filename), fs.fileTitle(spr.filename))
logFile:write("\nOutput folder set to: " .. output_folder)
local image_n = 0
local tileset_n = 0

local function write_json_data(filename, data)
  logFile:write("\n[write_json_data] Entered function for filename: " .. filename)
  logFile:write("\n[write_json_data] Loading json.lua library.")
  local json = dofile('./json.lua')
  logFile:write("\n[write_json_data] Opening file for writing: " .. filename)
  local file = io.open(filename, "w")
  logFile:write("\n[write_json_data] Encoding data to JSON format.")
  file:write(json.encode(data))
  file:close()
  logFile:write("\n[write_json_data] File closed. Exiting function.")
end

local function fill_user_data(t, obj)
  if obj.color.alpha > 0 then
    logFile:write("\n  [fill_user_data] Object has a color. Alpha: " .. obj.color.alpha)
    if obj.color.alpha == 255 then
      t.color = string.format("#%02x%02x%02x",
                              obj.color.red,
                              obj.color.green,
                              obj.color.blue)
    else
      t.color = string.format("#%02x%02x%02x%02x",
                              obj.color.red,
                              obj.color.green,
                              obj.color.blue,
                              obj.color.alpha)
    end
    logFile:write("\n  [fill_user_data] Color formatted as: " .. t.color)
  end
  if pcall(function() return obj.data end) then -- a tag doesn't have the data field pre-v1.3
    logFile:write("\n  [fill_user_data] Checking for user data property.")
    if obj.data and obj.data ~= "" then
      logFile:write("\n  [fill_user_data] User data found: " .. obj.data)
	  -- note that the data here should aready be encoded to json to be able to parsed again elsewhere.
	  -- if not, it will become one big string, where the value is double escaped.
      t.data = obj.data
    else
      logFile:write("\n  [fill_user_data] No user data or it is empty.")
    end
  end
  logFile:write("\n  [fill_user_data] Exiting.")
end

local function export_tileset(tileset)
  logFile:write("\n [export_tileset] Entered.")
  local t = {}
  local grid = tileset.grid
  local size = grid.tileSize
  t.grid = { tileSize={ width=grid.tileSize.width, height=grid.tileSize.height } }
  logFile:write("\n [export_tileset] Grid size: " .. size.width .. "x" .. size.height)
  if #tileset > 0 then
    logFile:write("\n [export_tileset] Tileset has " .. #tileset .. " tiles. Processing image.")
    local spec = spr.spec
    spec.width = size.width
    spec.height = size.height * #tileset
    local image = Image(spec)
    image:clear()
    logFile:write("\n [export_tileset] Created new image of size " .. spec.width .. "x" .. spec.height)
    for i = 0,#tileset-1 do
      local tile = tileset:getTile(i)
      image:drawImage(tile, 0, i*size.height)
    end
    logFile:write("\n [export_tileset] All tiles drawn to the new image.")

    tileset_n = tileset_n + 1
    local imageFn = fs.joinPath(output_folder, "tileset" .. tileset_n .. ".png")
    logFile:write("\n [export_tileset] Saving image to: " .. imageFn)
    image:saveAs(imageFn)
    t.image = imageFn
  else
    logFile:write("\n [export_tileset] Tileset is empty.")
  end
  logFile:write("\n [export_tileset] Exiting.")
  return t
end

local function export_tilesets(tilesets)
  logFile:write("\n[export_tilesets] Entered. Found " .. #tilesets .. " tilesets.")
  local t = {}
  for i,tileset in ipairs(tilesets) do
    logFile:write("\n[export_tilesets] Exporting tileset #" .. i)
    table.insert(t, export_tileset(tileset))
  end
  logFile:write("\n[export_tilesets] Exiting.")
  return t
end

local function export_frames(frames)
  logFile:write("\n[export_frames] Entered. Found " .. #frames .. " frames.")
  local t = {}
  for i,frame in ipairs(frames) do
    logFile:write("\n  [export_frames] Processing frame #" .. i .. " with duration " .. frame.duration)
    table.insert(t, { duration=frame.duration })
  end
  logFile:write("\n[export_frames] Exiting.")
  return t
end

local function export_cel(cel)
  logFile:write("\n    [export_cel] Entered for frame " .. (cel.frameNumber-1))
  local t = {
    frame=cel.frameNumber-1,
    bounds={ x=cel.bounds.x,
             y=cel.bounds.y,
             width=cel.bounds.width,
             height=cel.bounds.height }
  }
  logFile:write("\n    [export_cel] Cel bounds: x="..t.bounds.x..", y="..t.bounds.y..", w="..t.bounds.width..", h="..t.bounds.height)

  if cel.image.colorMode == ColorMode.TILEMAP then
    logFile:write("\n    [export_cel] Cel is a Tilemap.")
    local tilemap = cel.image
    t.tilemap = { width=tilemap.width,
                  height=tilemap.height,
                  tiles={} }
    logFile:write("\n    [export_cel] Exporting " .. (tilemap.width*tilemap.height) .. " tiles.")
    for it in tilemap:pixels() do
      table.insert(t.tilemap.tiles, pc.tileI(it()))
    end
  else
    logFile:write("\n    [export_cel] Cel is a regular image.")
    image_n = image_n + 1
    local imageFn = fs.joinPath(output_folder, "image" .. image_n .. ".png")
    logFile:write("\n    [export_cel] Saving image to: " .. imageFn)
    cel.image:saveAs(imageFn)
    t.image = imageFn
  end

  fill_user_data(t, cel)
  logFile:write("\n    [export_cel] Exiting.")
  return t
end

local function export_cels(cels)
  logFile:write("\n  [export_cels] Entered. Found " .. #cels .. " cels.")
  local t = {}
  for _,cel in ipairs(cels) do
    table.insert(t, export_cel(cel))
  end
  logFile:write("\n  [export_cels] Exiting.")
  return t
end

local function get_tileset_index(layer)
  logFile:write("\n    [get_tileset_index] Searching for tileset index for layer: " .. layer.name)
  for i,tileset in ipairs(layer.sprite.tilesets) do
    if layer.tileset == tileset then
      logFile:write("\n    [get_tileset_index] Found matching tileset at index " .. (i-1))
      return i-1
    end
  end
  logFile:write("\n    [get_tileset_index] No matching tileset found. Returning -1.")
  return -1
end

local function export_layer(layer, export_layers)
  logFile:write("\n  [export_layer] Entered for layer: " .. layer.name)
  local t = { name=layer.name }
  if layer.isImage then
    logFile:write("\n  [export_layer] Layer is an Image layer.")
    if layer.opacity < 255 then
      t.opacity = layer.opacity
      logFile:write("\n  [export_layer] Opacity is not default: " .. layer.opacity)
    end
    if layer.blendMode ~= BlendMode.NORMAL then
      t.blendMode = layer.blendMode
      logFile:write("\n  [export_layer] Blend mode is not default: " .. layer.blendMode)
    end
    if #layer.cels >= 1 then
      logFile:write("\n  [export_layer] Layer has cels, starting export.")
      t.cells = export_cels(layer.cels)
    end
    if pcall(function() return layer.isTilemap end) then
      if layer.isTilemap then
        logFile:write("\n  [export_layer] Layer is a Tilemap layer.")
        t.tileset = get_tileset_index(layer)
      end
    end
  elseif layer.isGroup then
    logFile:write("\n  [export_layer] Layer is a Group. Recursing into sub-layers.")
    t.layers = export_layers(layer.layers)
  end
  fill_user_data(t, layer)
  logFile:write("\n  [export_layer] Exiting for layer: " .. layer.name)
  return t
end

local function export_layers(layers)
  logFile:write("\n[export_layers] Entered. Found " .. #layers .. " layers.")
  local t = {}
  for _,layer in ipairs(layers) do
    table.insert(t, export_layer(layer, export_layers))
  end
  logFile:write("\n[export_layers] Exiting.")
  return t
end

local function ani_dir(d)
  local values = { "forward", "reverse", "pingpong" }
  local dir = values[d+1]
  logFile:write("\n    [ani_dir] Converted direction " .. d .. " to '" .. dir .. "'.")
  return dir
end

local function export_tag(tag)
  logFile:write("\n  [export_tag] Entered for tag: " .. tag.name)
  local t = {
    name=tag.name,
    from=tag.fromFrame.frameNumber-1,
    to=tag.toFrame.frameNumber-1,
    aniDir=ani_dir(tag.aniDir)
  }
  logFile:write("\n  [export_tag] Frames: from " .. t.from .. " to " .. t.to)
  fill_user_data(t, tag)
  logFile:write("\n  [export_tag] Exiting for tag: " .. tag.name)
  return t
end

local function export_tags(tags)
  logFile:write("\n[export_tags] Entered. Found " .. #tags .. " tags.")
  local t = {}
  for _,tag in ipairs(tags) do
    table.insert(t, export_tag(tag, export_tags))
  end
  logFile:write("\n[export_tags] Exiting.")
  return t
end

local function export_slice(slice)
  logFile:write("\n  [export_slice] Entered for slice: " .. slice.name)
  local t = {
    name=slice.name,
    bounds={ x=slice.bounds.x,
             y=slice.bounds.y,
             width=slice.bounds.width,
             height=slice.bounds.height }
  }
  logFile:write("\n  [export_slice] Slice bounds: x="..t.bounds.x..", y="..t.bounds.y..", w="..t.bounds.width..", h="..t.bounds.height)
  if slice.center then
    t.center={ x=slice.center.x,
               y=slice.center.y,
               width=slice.center.width,
               height=slice.center.height }
    logFile:write("\n  [export_slice] Slice has center data.")
  end
  if slice.pivot then
    t.pivot={ x=slice.pivot.x,
               y=slice.pivot.y }
    logFile:write("\n  [export_slice] Slice has pivot data.")
  end
  fill_user_data(t, slice)
  logFile:write("\n  [export_slice] Exiting for slice: " .. slice.name)
  return t
end

local function export_slices(slices)
  logFile:write("\n[export_slices] Entered. Found " .. #slices .. " slices.")
  local t = {}
  for _,slice in ipairs(slices) do
    -- The incorrect second argument has been removed.
    table.insert(t, export_slice(slice,export_slices))
  end
  logFile:write("\n[export_slices] Exiting.")
  return t
end

----------------------------------------------------------------------
-- Creates output folder
logFile:write("\n\n--- MAIN EXECUTION ---")
logFile:write("\nCreating output directory: " .. output_folder)
fs.makeDirectory(output_folder)
logFile:write("\nOutput directory created or already exists.")

----------------------------------------------------------------------
-- Write /sprite.json file in the output folder

local jsonFn = fs.joinPath(output_folder, "sprite.json")
logFile:write("\nPreparing to write main JSON file: " .. jsonFn)
logFile:write("\nBuilding main data object...")
local data = {
  filename=spr.filename,
  width=spr.width,
  height=spr.height,
  frames=export_frames(spr.frames),
  layers=export_layers(spr.layers)
}
logFile:write("\n...Base data object created with filename, dimensions, frames, and layers.")

if #spr.tags > 0 then
  logFile:write("\nSprite has tags. Exporting them.")
  data.tags = export_tags(spr.tags)
else
  logFile:write("\nSprite has no tags.")
end

if #spr.slices > 0 then
  logFile:write("\nSprite has slices. Exporting them.")
  data.slices = export_slices(spr.slices)
else
  logFile:write("\nSprite has no slices.")
end

if pcall(function() return spr.tilesets end) then
  logFile:write("\nSprite has tilesets property. Exporting them.")
  data.tilesets = export_tilesets(spr.tilesets)
else
  logFile:write("\nSprite does not have tilesets property.")
end

logFile:write("\nData object fully built. Writing to JSON file.")
write_json_data(jsonFn, data)
logFile:write("\n\n--- EXPORT COMPLETE ---")
logFile:close()
