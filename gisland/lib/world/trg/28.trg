#2800
Veteran transport~
2 q 100
~
if %actor.level% >= 2 || %actor.has_item(3203)% 
%send% %actor% You already completed Hunter exam before.
%teleport% %actor% 1406
%force% %actor% look
halt
end
if %actor.level% >= 2 && %direction% /= north
%load% obj 3037 %actor% light
%send% %actor% You already completed Hunter exam before.
%teleport% %actor% 2813
%force% %actor% down
end
~
#2801
Newbie receive equips~
2 g 100
~
if %direction% == south && !%actor.eq(*)%
%load% obj 3037 %actor% light
~
#2802
test mode~
2 c 100
test_mode~
%send% %actor% Test mode: ON
nop %actor.hunger(-1)%
nop %actor.thirst(-1)%
~
#2803
no book~
2 c 100
book~
%send% %actor% Nani!?!
~
#2804
who block~
2 c 100
wh~
%send% %actor% Who not available while in Hunter Exams.
~
#2805
quit return hschool~
1 c 3
quit~
if %cmd.mudcommand% == quit
if %actor.room.vnum% == 1401
%teleport% %actor% 2800
%force% %actor% quit
elseif %actor.room.vnum% == 1407
%force% %actor% rent
else
%send% %actor% 	CYou only can save your items in front of Elena, otherwise type 	Rforcequit	C.	n
end
else
return 0
end
~
#2806
clear room~
2 abs 100
~
wait 0.1 sec
if %actor.has_item(1405)% || %actor.has_item(1406)% || %actor.has_item(1407)%
%send% %actor% Loading...
%teleport% %actor% 1407
%force% %actor% give all.token elena
%force% %actor% down
elseif %actor.level% >= 2 || %actor.has_item(3203)%
%send% %actor% Loading...
%teleport% %actor% 1407
%force% %actor% down
end
~
#2807
beans greet~
0 e 0
has entered the game.~
wait 2 sec
if %actor.level% <= 1 && %actor.room% /= %self.room%
say Hello candidate number %actor.id%
wait 4 sec
say You might be %actor.name%-sama, do you?
wait 4 sec
say Go 	Ynorth	n to learn more at Hunter exams!
wait 4 sec
say or go down if you already played MUD before.
wait 4 sec
smile %actor.name%
~
#2808
beans advice~
0 g 100
~
wait 2 sec
say Are you lost %actor.name%-sama?
wait 4 sec
say Go 	Ynorth	n to learn more at Hunter exams!
wait 4 sec
say or go down if you already played MUD before.
wait 4 sec
smile %actor.name%
~
#2809
gain gold coins card~
1 c 3
gai~
if %self.carried_by% == %actor%
%send% %actor% You must hold something before gain it.
halt
end
eval hold %actor.eq(hold)%
if %cmd%==gain && %hold.id% /= %self.id%
%force% %actor% say GAIN!
wait 1 sec
%send% %actor% Your %self.shortdesc% turns into 1000 gold coins. type <gold>
%echoaround% %actor% %actor.name%'s %self.shortdesc% turns into a lot of gold coins.
nop %actor.gold(1000)%
%purge% %self%
else
return 0
end
~
#2813
receive ring~
2 q 100
~
if %direction% == down && !%actor.eq(rfinger)%
%load% obj 3202 %actor% inv
end
~
#2814
gold coins card~
2 q 100
~
if %direction% == east && !%actor.varexists(coins_card_collected)%
%load% obj 2809 %actor% inv
eval coins_card_collected 1
remote coins_card_collected %actor.id%
end
~
$~
