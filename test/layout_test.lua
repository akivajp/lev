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

sys = lev.system()
screen = sys:set_video_mode(640, 480)
--screen:clear(lev.prim.red)
screen:draw(img, 50, 50)
print(screen:draw(lay, 5, 5, 128))
screen:flip()
sys:run()

