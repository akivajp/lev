require 'lev.font'
require 'lev.image'

f = lev.font.load()
f.size = 72

--img = lev.image.string(f, 'Hello, World! ggg 日本語', lev.prim.color('red'), lev.prim.color('white'), 2)
img = lev.image.string(f, 'Hello, World! ggg 日本語', lev.prim.color('red'), lev.prim.color('white'), 10)
img:save('test.bmp')

