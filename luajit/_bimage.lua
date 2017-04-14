local C = ffi.load"bimage"

local mod = {}

mod.OK = C.BIMAGE_OK
mod.ERR = C.BIMAGE_ERR
mod.GRAY8 = C.GRAY8
mod.GRAY16 = C.GRAY16
mod.GRAY32 = C.GRAY32
mod.RGB32 = C.RGB24
mod.RGB48 = C.RGB48
mod.RGB96 = C.RGB96
mod.RGBA32 = C.RGBA32
mod.RGBA64 = C.RGBA64
mod.RGBA128 = C.RGBA128

local function makeType(channels, depth)
    local b = ffi.new("BIMAGE_TYPE[1]")
    C.bimageMakeType(channels, depth, b)
    return b[0]
end
mod.makeType = makeType

local function createImage(width, height, t)
    return ffi.gc(C.bimageCreate(width, height, t or mod.RGBA32), C.bimageRelease)
end

local function createImageOnDisk(name, width, height, t)
    return ffi.gc(C.bimageCreateOnDisk(name, width, height, t or RGBA32), C.bimageRelease)
end

local function getPixel(im, x, y)
    local b = ffi.new("bpixel[1]")

    if C.bimageGetPixel(im, x, y, b) == C.BIMAGE_ERR then
        return nil
    end

    return b[0]
end

local function setPixel(im, x, y, p)
    return C.bimageSetPixel(im, x, y, p)
end

local function createPixel(r, g, b, a, d)
    return C.bpixelCreate(r, g or r, b or r, a or 0, d or -1)
end

local pixel_functions = {
    copy = function(a)
        return createPixel(a.data[0], a.data[1], a.data[2], a.data[3], a.depth)
    end,
    new = createPixel,
    to_table = function(a) return {a.data[0], a.data[1], a.data[2], a.data[3]} end
}

local pixel
local pixel_mt = {
    __add = function(a, b)
        local px = a:copy()
        for i = 0, 3 do
            px.data[i] = px.data[i] + b.data[i]
            if px.data[i] > C.bimageTypeMax(makeType(1, a.depth)) then
                px.data[i] = C.bimageTypeMax(makeType(1, a.depth))
            end
        end
        return px
    end,
    __sub = function(a, b)
        local px = a:copy()
        for i = 0, 3 do
            px.data[i] = px.data[i] - b.data[i]
            if px.data[i] < 0 then
                px.data[i] = 0
            end
        end
        return px
    end,
    __tostring = function(px)
        return px.data[0] .. ", " .. px.data[1] .. ", " .. px.data[2] .. ", " .. px.data[3]
    end,
    __call = function(px, x)
        return px.data[x % 4]
    end,
    __index = pixel_functions

}
mod.pixel = ffi.metatype("bpixel", pixel_mt)

local image_functions = {
    create = createImage,
    createOnDisk = createImageOnDisk,
    crop = C.bimageCrop,
    grayscale = C.bimageGrayscale,
    resize = C.bimageResize,
    hash = C.bimageHash,
    filter = C.bimageFilter,
    open = C.bimageOpen,
    open16 = C.bimageOpen16,
    save = C.bimageSave,
    convertDepth = C.bimageConvertDepth,
    convertChannels = C.bimageConvertChannels,
    get = function(im, x, y)
        return getPixel(im, x, y)
    end,
    set = function(im, x, y, p)
        setPixel(im, x, y, p)
    end,
    consume = function(self, other)
        local x = ffi.new("bimage*[1]")
        return C.bimageConsume(x, other)
    end,
    iter = function(im, x, y, w, h)
        x = x or 0
        y = y or 0
        w = w or im.width
        h = h or im.height
        local i, j = x-1, y
        return function()
            if i == w - 1 then
                j = j + 1
                i = 0
            else
                i = i + 1
            end

            if j == h then
                return nil
            else
                return i, j, getPixel(im, i, j)
            end
        end
    end
}

local image
local image_mt = {
    __call = function(im, x)
        return getPixel(im, x[1], x[2])
    end,
    __tostring = function(im)
        return "<image: " .. im.width .. "x" .. im.height .. "; " .. im.type .. ">"
    end,
    __index = image_functions,
    __newindex = function(self, idx, px)
        setPixel(self, idx[1], idx[2], px)
    end
}
mod.image = ffi.metatype("bimage", image_mt)


return mod
