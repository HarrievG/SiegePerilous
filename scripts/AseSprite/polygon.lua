--[[
Aseprite Script: Interactive Box2D Polygon Exporter
Generates and visualizes a simplified polygon outline for each frame.
The UI is inspired by the Aseprite API path editor example, providing
a live preview canvas and interactive controls.

ALGORITHMS USED:
1. Moore-Neighbor Tracing: To find the initial pixel-perfect outline.
2. Ramer-Douglas-Peucker: To simplify the polygon for performance.
--]]

-- //////////////////////////////////////////////////////////////////////////
-- // 1. CORE ALGORITHMS (Verified and Corrected)
-- //////////////////////////////////////////////////////////////////////////

--[[
  Finds the outline of non-transparent pixels in an image using Moore-Neighbor Tracing.
  @param image The Aseprite image object to trace.
  @return A table of ordered points { {x=x1, y=y1}, ... } or nil if empty.
--]]

 local POLYGON_DATA_KEY = "polygonCollisionShape"
 
local function findOutline(image)
    local startPoint = nil
    local boundary = {}
    local w, h = image.width, image.height

    for y = 0, h - 1 do
        for x = 0, w - 1 do
            if app.pixelColor.rgbaA(image:getPixel(x, y)) > 0 then
                startPoint = {x = x, y = y}
                table.insert(boundary, startPoint)
                goto found_start_point
            end
        end
    end
    ::found_start_point::

    if not startPoint then return nil end

    local currentPoint = startPoint
    local neighbors = {
      {-1, 0}, {-1, -1}, {0, -1}, {1, -1},
      {1, 0}, {1, 1}, {0, 1}, {-1, 1}
    }
    local backtrackIndex = 4

    repeat
        local foundNext = false
        for i = 0, 7 do
            local index = (backtrackIndex + i) % 8
            local neighborOffset = neighbors[index + 1]
            local nextX, nextY = currentPoint.x + neighborOffset[1], currentPoint.y + neighborOffset[2]

            if nextX >= 0 and nextX < w and nextY >= 0 and nextY < h and app.pixelColor.rgbaA(image:getPixel(nextX, nextY)) > 0 then
                currentPoint = {x = nextX, y = nextY}
                table.insert(boundary, currentPoint)
                backtrackIndex = (index + 5) % 8
                foundNext = true
                break
            end
        end
        if not foundNext then break end
    until (currentPoint.x == startPoint.x and currentPoint.y == startPoint.y)

    table.remove(boundary)
    return boundary
end

--[[
  Simplifies a polygon using the Ramer-Douglas-Peucker algorithm.
  This version uses safe table manipulation to avoid `unpack` limits.
--]]
local function ramerDouglasPeucker(points, epsilon)
    if #points < 3 then return points end

    local function slice(tbl, first, last)
        local newTbl = {}
        for i = first, last do table.insert(newTbl, tbl[i]) end
        return newTbl
    end

    local function concat(t1, t2)
        for i = 1, #t2 do table.insert(t1, t2[i]) end
        return t1
    end

    local function getPerpendicularDistance(p, p1, p2)
        local dx = p2.x - p1.x
        local dy = p2.y - p1.y
        if dx == 0 and dy == 0 then
            return math.sqrt((p.x - p1.x)^2 + (p.y - p1.y)^2)
        end
        local t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / (dx^2 + dy^2)
        t = math.max(0, math.min(1, t))
        local projX = p1.x + t * dx
        local projY = p1.y + t * dy
        return math.sqrt((p.x - projX)^2 + (p.y - projY)^2)
    end

    local dmax = 0
    local index = 0
    local last = #points
    for i = 2, last - 1 do
        local d = getPerpendicularDistance(points[i], points[1], points[last])
        if d > dmax then
            index = i
            dmax = d
        end
    end

    if dmax > epsilon then
        local recResults1 = ramerDouglasPeucker(slice(points, 1, index), epsilon)
        local recResults2 = ramerDouglasPeucker(slice(points, index, last), epsilon)
        table.remove(recResults2, 1)
        return concat(recResults1, recResults2)
    else
        return {points[1], points[last]}
    end
end


-- //////////////////////////////////////////////////////////////////////////
-- // 2. ASEPRITE SCRIPT UI AND MAIN LOGIC
-- //////////////////////////////////////////////////////////////////////////

local spr = app.sprite
if not spr then return app.alert("Please open a sprite first.") end

local activeLayer = app.activeLayer
if not activeLayer or not activeLayer.isEditable then return app.alert("Please select an editable layer.") end

-- Use theme colors for drawing on the canvas
local PointColor = app.theme.color["text"]
local LineColor = app.theme.color["selected"]
local PointRadius = 3
local CanvasPadding = 20

-- Cache for generated data to avoid recalculating on every repaint
local cache = {
    outline = {},
    simplified = {}
}


    -- Create the dialog
local dlg = Dialog("Interactive Box2D Exporter")

-- Main function to update the preview and text output
local function updatePreview(dlg)
    local data = dlg.data
    local frameNumber = data.frame
    local epsilon = data.epsilon

    -- Clear cache for this frame if epsilon has changed
    if cache.simplified[frameNumber] and cache.simplified[frameNumber].epsilon ~= epsilon then
        cache.simplified[frameNumber] = nil
    end
    local cel = activeLayer:cel(frameNumber)
    if not cel then
        dlg.data.polygonData = "No cel on this frame."
        dlg:repaint()
        return
    end

    local image = cel.image

    -- Get outline (from cache or generate new)
    if not cache.outline[frameNumber] then
        cache.outline[frameNumber] = findOutline(image)
    end
    local outline = cache.outline[frameNumber]

    if not outline or #outline < 2 then
        dlg.data.polygonData = "-- No outline found on this frame."
        dlg:repaint()
        return
    end

    -- Get simplified polygon (from cache or generate new)
    if not cache.simplified[frameNumber] then
        local simplified = ramerDouglasPeucker(outline, epsilon)
        cache.simplified[frameNumber] = { points = simplified, epsilon = epsilon }
    end

    -- Generate Lua output string
    local simplifiedPoints = cache.simplified[frameNumber].points
    local output = "{\n"
    for _, p in ipairs(simplifiedPoints) do
        local centeredX = p.x - image.width / 2
        local centeredY = -(p.y - image.height / 2) -- Box2D Y-axis is often inverted
        output = output .. "  {x = " .. string.format("%.2f", centeredX) .. ", y = " .. string.format("%.2f", centeredY) .. "},\n"
    end
    output = output .. "}"
    local jsonStr = json.encode(simplifiedPoints)
    dlg.data.polygonData =  json.encode(simplifiedPoints)

    -- Request the dialog to repaint the canvas
    dlg:repaint()
end

local function savePolygonToCel(cel, polygon)
	if not cel or not polygon then return end
	local dataObj = {}
	if cel.data and cel.data ~= "" then
		local success, decoded = pcall(json.decode, cel.data)
		if success and type(decoded) == "table" then
			dataObj = decoded
		end
	end
	dataObj[POLYGON_DATA_KEY] = polygonToString(polygon)
	cel.data = json.encode(dataObj)
	app.refresh()
end
	
do
    local json = dofile('./json.lua')
    local get_distance = function(a, b)
        return math.sqrt((b.x - a.x) ^ 2 + (b.y - a.y) ^ 2)
    end
        local mouse = Point(0, 0)
    -- The `onpaint` event handler for the canvas
    local function onPaintCanvas(ev)
        local g = ev.context
        local data = dlg.data
        local frameNumber = data.frame
    
        -- Draw background
        g:drawThemeRect("sunken_focused", Rectangle(0, 0, g.width, g.height))
    
        local simplified = cache.simplified[frameNumber] and cache.simplified[frameNumber].points
        if not simplified or #simplified < 2 then return end
    
        local cel = activeLayer:cel(frameNumber)
        if not cel then return end
    
        -- Calculate scale and offset to fit the polygon in the canvas
        local imgW, imgH = cel.image.width, cel.image.height
        local scaleX = (g.width - CanvasPadding * 2) / imgW
        local scaleY = (g.height - CanvasPadding * 2) / imgH
        local scale = math.min(scaleX, scaleY)
        local offsetX = (g.width - imgW * scale) / 2
        local offsetY = (g.height - imgH * scale) / 2
    
        local function transformPoint(p)
            return { x = p.x * scale + offsetX, y = p.y * scale + offsetY }
        end
    
        -- Draw the polygon lines
        g.color = LineColor
        g:beginPath()
        local firstPoint = transformPoint(simplified[1])
        g:moveTo(firstPoint.x, firstPoint.y)
        for i = 2, #simplified do
            local p = transformPoint(simplified[i])
            g:lineTo(p.x, p.y)
        end
        g:closePath()
        g:stroke()
    
        --- redo 
        -- Draw the vertices
        g.color = PointColor

        for _, p in ipairs(simplified) do
            local point = transformPoint(p)
            g:beginPath()
            local pointShape = Rectangle {
                x = point.x - PointRadius,
                y = point.y - PointRadius,
                width = PointRadius * 2,
                height = PointRadius * 2
            }
            g:roundedRect(pointShape, PointRadius, PointRadius)
            local isMouseOver = get_distance(mouse, point) < PointRadius
            local isLineStart = point == lineStart
            if isMouseOver or isLineStart then
                g.color = HighlightColor
                g:fill()
            else
                g.color = RegularColor
                g:stroke()
            end
        end
    end


    dlg:slider{
        id = "frame",
        label = "Frame:",
        min = 1,
        max = #spr.frames,
        value = app.frame.frameNumber,
        onchange = function() updatePreview(dlg) end
    }
    dlg:number{
        id = "epsilon",
        label = "Simplification:",
        text = "1.5",
        decimals = 1,
        onchange = function() updatePreview(dlg) end
    }
    dlg:canvas{
        id = "canvas",
        width = 250,
        height = 250,
        onpaint = onPaintCanvas
    }
    
    dlg:label{
        id = "polygonData",
        label = "Data:",
        text = ""
    }
    --dlg:entry{
    --    id = "polygonData",
    --    minheight = 150,
    --    readonly = true,
    --    monospace = true,
    --    wordwrap = false
    --}
    
    dlg:button{
        id = "save ALL",
        text = "Save all polygons to cells",
        onclick = function(ev)
            local g = ev.context
            local data = dlg.data
            local frameNumber = data.frame
            local jsonStr = json.encode(cache.simplified[frameNumber].points)
            print (jsonStr)
            --app.setClipboardText(dlg.data.polygonData)
            app.alert("Copied!")
        end
    }  
    dlg:button{
        id = "save",
        text = "Save to cell",
        onclick = function(ev)
            local g = ev.context
            local data = dlg.data
            local frameNumber = data.frame
            local jsonStr = json.encode(cache.simplified[frameNumber].points)
            print (jsonStr)
			
			local cel = app.cel
			local dataObj = {}
			dataObj[POLYGON_DATA_KEY] = cache.simplified[frameNumber].points
			--jsonStr;
			cel.data = json.encode(dataObj)
			app.refresh()
            --app.setClipboardText(dlg.data.polygonData)
            app.alert("Copied!")
        end
    }
    dlg:button{
        id = "close",
        text = "Close",
        focus = true,
        onclick = function() dlg:close() end
    }
    
    -- Show the dialog as non-modal and run the initial preview
    dlg:show{ wait = true }
    updatePreview(dlg)
end