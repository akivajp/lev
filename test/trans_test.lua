require 'lev.std'
require 'debug'

screen = lev.screen()

img1 = lev.bitmap(640, 480)
img1:fill_rect(50, 50, 320, 240, lev.color(255, 0, 0, 180))
img1:fill_rect(100, 100, 320, 240, lev.color(0, 0, 255, 180))
img2 = lev.bitmap(648, 480)
img2:clear(lev.color(255, 255, 0))
tran = lev.transition()
tran:set_current(img1)
tran:set_next(img2, 5, 'cross_fade')
tran:set_next(img1, 5, 'cross_fade')
tran:texturize()

system.on_tick = function()
--  print('ticks: ', system.ticks)
  screen:clear()
  screen:draw(tran)
  screen:swap()
end

system:run()

