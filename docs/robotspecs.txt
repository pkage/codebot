robot specs
-----------

cb.pkbasic specially addresses variables like so:

Var | Value
----+----------
#A  | last scan result [0-1] [wall/robot]
#B  | last scan distance
#C  | last movement result [0-1] [fail/success]
#E  | Robot's energy [readonly*]
#H  | Robot's health [readonly*]
#X  | Robot's X position on map [readonly*]
#Y  | Robot's Y position on map [readonly*]

cb.pkbasic also has special keywords:

Keyword   |   Value
----------+-------------
```move```| moves in a direction specified **
```scan```| scans in a direction specified **
```fire```| fires in a direction specified **


notes
_* readonly values are updated every tick, so any changes you make will by overwritten by the controller's values for that bot_
_** valid directions are up/down/left/right_

