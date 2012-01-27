require 'lev.std'

window = system:window()
screen = window:screen()

tran = lev.image.transition('bbb.png')
tran:set_next('test.png', 3, 'cross_fade')
tran:texturize()

system.on_tick = function()
  print('ticks: ', system.ticks)
  screen:clear()
  screen:draw(tran)
  screen:swap()
end

system:run()

system:close()

