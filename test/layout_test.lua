require 'lev.std'

lay = lev.image.layout(600)
local f = lev.font.load('default.ttf')
if f then
  lay.font = f
  lay.font.size = 30
  lay.ruby = f:clone()
  lay.ruby.size = 15
end
lay.spacing = 3

lay:reserve_word('日本語', 'にほんご')
lay.color = lev.prim.red
lay:reserve_word('Hello, World!')
lay:reserve_new_line()
lay:reserve_word('newline!')
lay:complete()

window = system:window('Layout Test', 640, 480)
screen = window:screen()

img1 = lev.image.create(320, 240)
img1:clear(lev.prim.color(255, 0, 0))
img1:texturize()
img2 = img1:clone()
img2:clear(lev.prim.color(0, 255, 0))
img2:texturize()
img3 = img1:clone()
img3:clear(lev.prim.color(0, 0, 255))
img3:texturize()

local start = system.ticks
local count = 1
system.on_tick = function()
  screen:clear();
  screen:draw(0, 0, tex, 128)
  screen:draw(lay, 10, 10, 200)
  screen:draw(img1, 100, 100, 128)
  screen:draw(img2, 150, 150, 128)
  screen:draw(img3, 200, 200, 128)
  screen:swap()
end

system:run()

