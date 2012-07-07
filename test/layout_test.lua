require 'lev.std'
require 'debug'

lay = lev.layout(600)
local f = lev.font('fonts/default.ttf')
if f then
  lay.font = f
  lay.font.size = 30
  lay.ruby = f:clone()
  lay.ruby.size = 15
end

lay:reserve_word('日本語', 'にほんご')
lay.color = lev.prim.red
lay:reserve_word('Hello, World!')
lay:reserve_new_line()
lay:reserve_word('newline!')
lay:complete()

screen = lev.screen('Layout Test', 640, 480)


img1 = lev.bitmap(320, 240)
img1:clear(lev.color(255, 0, 0))
img1:texturize()
img2 = img1:clone()
img2:clear(lev.color(0, 255, 0))
img2:texturize()
img3 = img1:clone()
img3:clear(lev.color(0, 0, 255))
img3:texturize()

lay:texturize()
sw = lev.stop_watch()
system.on_tick = function()
  sw:start()
  screen:clear();
  screen:draw(lay, 10, 10, 200)
  screen:draw(img1, 100, 100, 128)
  screen:draw(img2, 150, 150, 128)
  screen:draw(img3, 200, 200, 128)
  screen:swap()
  print('TIME', sw.time)
end

system:run()

