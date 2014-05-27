#a = 0
label 1
#z = 0
if #a == #z
scan left
#z = 1
if #a == #z
scan right
#z = 5
if #B < #z
goto 2
#z = 0
if #a == #z
move left
if #a != #z
move right
goto 1
label 2
#z = 0
if #a == #z
goto 3
#z = 1
if #a == #z
goto 4
label 3
#a = 1
goto 1
label 4
#a = 0
goto 1
