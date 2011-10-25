require 'lev.image'
require 'lev.system'

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

sys = lev.system()
screen = sys:set_video_mode(640, 480, 32)

local start = sys.ticks
local count = 1
sys.on_tick = function()
local current = sys.ticks
--  local start = sys.ticks
--  screen:clear();
  screen:clear(255, 0, 0, 0);
--  screen:clear(lev.prim.color('red'));
--  print('clear:', sys.ticks - start)
--  screen:blit(0, 0, img) 
--  screen:draw(0, 0, img, 128)
--  screen:blit(img3)
--  screen:blit(100, 50, img2, 200)
--  screen:draw(lay, 10, 10, 200)
--  screen:flip()
  screen:swap()
print('now', (sys.ticks - current))
print('ave', (sys.ticks - start) / count)
count = count + 1
end

sys.on_quit = function(e)
  sys:quit(true)
end

sys.on_motion = function(e)
--  print('motion: ', e.left, e.right, e.middle)
--  print('motion!', e.x, e.y, e.dx, e.dy)
end

sys.on_left_down = function(e)
  print('left', e.x, e.y, e.pressed, e.released)
end

sys.on_middle_down = function(e)
  print('middle down', e.x, e.y, e.pressed, e.released)
end

sys.on_key_down = function(e)
  print('keydown!', e.key, e.key_code)
end

sys.on_key_up = function(e)
  print('keyup!')
end

sys:run()

--screen:save('aaa.bmp')

