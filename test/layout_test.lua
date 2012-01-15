require 'lev.std'

lay = lev.image.layout(600)
lay.font.size = 30
lay.ruby.size = 15
lay.spacing = 3
print(lay:reserve_word('日本語', 'にほんご'))
lay.color = lev.prim.red
print(lay:reserve_word('Hello, World!'))
print(lay:reserve_new_line())
print(lay:reserve_word('newline!'))
print(lay:complete())

--txt = lev.image.string('aaa')
--img = lev.image.create(640, 480)
--print(txt)
--print(img:draw(txt))
--img:save('test.bmp')

img = lev.image.load('test.png')
--img = lev.image.load('test.bmp')
print(img)

img2 = lev.image.load('levana3.png')
img3 = lev.image.create(640, 480)
img3:fill_rect(5, 5, 300, 200, lev.prim.color('blue', 128))

window = system:window('Layout Test', 640, 480)
screen = window:screen()

local start = system.ticks
local count = 1
tex = img:texturize()
print(tex)
system.on_tick = function()
  local current = system.ticks
  screen:clear();
  screen:draw(0, 0, tex, 128)
  screen:draw(lay, 10, 10, 200)
  screen:swap()
--  print('now', (system.ticks - current))
--  print('ave', (system.ticks - start) / count)
  count = count + 1
end

system.on_quit = function(e)
  system:quit(true)
end

system.on_motion = function(e)
--  print('motion: ', e.left, e.right, e.middle)
  print('motion!', e.x, e.y, e.dx, e.dy)
end

system.on_left_down = function(e)
  print('left down', e.x, e.y, e.pressed, e.released)
end

system.on_middle_down = function(e)
  print('middle down', e.x, e.y, e.pressed, e.released)
end

system.on_right_down = function(e)
  print('right down', e.x, e.y, e.pressed, e.released)
end

system.on_key_down = function(e)
  print('keydown!', e.key, e.key_code, e.scan_code)
end

system.on_key_up = function(e)
  print('keyup!')
end

system:run()

--screen:save('aaa.bmp')

