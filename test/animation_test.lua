require 'lev.std'
require 'debug'

screen = system:screen(640, 480)

anim = lev.animation()
local img = lev.bitmap(320, 240)
img:clear(lev.color(255, 0, 0))
anim:append(img)
img = img:clone()
img:clear(lev.color(0, 0, 255))
anim:append(img)

function system.on_tick()
  screen:clear()
  screen:draw(anim)
  screen:swap()
end

system:run(screen)

