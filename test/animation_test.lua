require 'lev.std'

window = system:window(640, 480)
screen = window:screen()

anim = lev.image.animation()
local img = lev.image.create(320, 240)
img:clear(lev.prim.color(255, 0, 0))
anim:append(img)
img = img:clone()
img:clear(lev.prim.color(0, 0, 255))
anim:append(img)

function system.on_tick()
  screen:clear()
  screen:draw(anim)
  screen:swap()
end

system:run()

