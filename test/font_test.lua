require 'lev.font'

f = lev.font.load() or os.exit(-1)
bitmap = f:rasterize('龍') or os.exit(-1)

for y = 0, bitmap.h do
  for x = 0, bitmap.w do
    print(x, y, bitmap:get_pixel(x, y))
  end
end

