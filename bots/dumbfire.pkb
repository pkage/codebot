#c = 25
// setup
label 2
#a = 0
if #b == 0
goto 3
if #b == 1
goto 4
label 3
#b = 0
goto 1
label 4
#b = 1
goto 1

// loop
label 1

if #b == 0
move up

if #b == 1
move down

#a += 1
fire left
fire right

if #a > #c
goto 2

goto 1
