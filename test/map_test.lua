require 'lev.system'
require 'lev.image'

system = lev.system()
window = system:window()
screen = window:screen()

img1 = lev.image.load('aaa.png')
img2 = lev.image.load('test.png')
map = lev.image.map()

str = lev.image.string(lev.font.load(), 'TEXT')
map:map_image(str, 0, 0)

local lclick = function()
  print('LEFT CLICKED!!!')
end

local hover = function()
  print('HOVER!')
end

--map:map_link(img1, img2, 50, 50, { on_left_click = lclick, on_hover = hover})
map:map_link(img1, img2, 50, 50, lclick, hover)
--map:map_link(img1, img2, 50, 50)
map:texturize()

system.on_motion = function(e)
--  print('x:', e.x, 'y:', e.y)
  map:on_hover(e.x, e.y)
end

system.on_left_down = function(e)
  print('x:', e.x, 'y:', e.y)
  map:on_left_click(e.x, e.y)
end

system.on_tick = function()
--  print('ticks:', system.ticks)
  screen:clear()
  screen:draw(map)
  screen:swap()
end

system:run()
system:close()

