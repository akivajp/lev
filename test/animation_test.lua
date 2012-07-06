require 'lev.std'
require 'debug'

screen = lev.screen(640, 480)

anim = lev.animation()
local img = lev.bitmap(320, 240)
img:clear(lev.color(255, 0, 0))
anim:append(img)
img = img:clone()
img:clear(lev.color(0, 0, 255))
anim:append(img)

function system.on_tick()
  print('TICK')
  screen:clear()
  print('TICK')
  screen:draw(anim)
  print('TICK')
  screen:swap()
  print('TICK')
end

system:run()

