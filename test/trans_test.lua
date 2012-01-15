require 'lev.std'

window = system:window()
screen = window:screen()

tran = lev.image.transition('bbb.png')
tran:set_next('test.png', 2000, 'cross_fade')
tran:texturize()

system.on_tick = function()
  print('tick: ', system.ticks)
  screen:clear()
  screen:draw(tran)
  screen:swap()
end

system:run()

