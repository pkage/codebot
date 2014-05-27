#a = 0
label 1
if #E > 40
//fire down
scan left
if #B < 5
#a = 1
if #a == 1
move right
if #a == 0
move left
goto 1
