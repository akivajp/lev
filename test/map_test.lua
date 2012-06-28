require 'lev.std'
require 'debug'

system:start_debug()
screen = system:screen()

img1 = lev.bitmap(320, 240)
img1:clear(lev.color(255, 0, 0))
img2 = img1:clone()
img2:clear(lev.color(0, 0, 255))
map = lev.map()

local on_lclick = function()
  lev.debug.print('on left click')
end

local on_hover = function()
  lev.debug.print('on hover')
end

map:map_link { img1, img2, 50, 50,
               on_lclick = on_lclick, on_hover = on_hover }

screen.on_motion = function(e)
--  lev.debug.print('motion x:' .. e.x ..  ' y:' .. e.y)
  map:on_hover(e.x, e.y)
end

screen.on_left_down = function(e)
  lev.debug.print('left click  x:' .. e.x .. ' y:' .. e.y)
  map:on_left_click(e.x, e.y)
end

screen.on_close = function()
  system:quit(true)
end

system.on_tick = function()
  screen:clear()
  screen:draw(map)
  screen:swap()
end

system:run()

