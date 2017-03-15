#901
snarl with glee~
0 g 100
~
wait 1 s
snarl
~
#902
glare~
0 g 100
~
wait 1 s
glare
~
#903
gain check~
1 c 1
gai~
if !%actor.varexists(book_purge_leaves)%
if %cmd% == gain
%send% %actor% You need call your "Book" before gain a card.
halt
else
return 0
end
end
~
#907
Flash Light~
0 g 100
~
wait 1 s
%echo% The Mri Treasure glows with a golden aura.
~
$~
