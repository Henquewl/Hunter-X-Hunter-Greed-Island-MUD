#40100
Sick villagers quest~
2 q 5
~
if %direction% == south
if %actor.is_pc%
%echo% A bunch of ninjas suddenly appears out the forest from east!
%load% mob 40100
%load% mob 40100
%load% mob 40100
%load% mob 40100
%load% mob 40100
%load% mob 40100
%door% 40103 east flags a
return 0
detach all %self.id%
end
end
if %direction% == north
if %actor.is_pc%
%echo% A bunch of ninjas suddenly appears out the forest from east!
%load% mob 40100
%load% mob 40100
%load% mob 40100
%load% mob 40100
%load% mob 40100
%load% mob 40100
%door% 40103 east flags a
return 0
detach all %self.id%
end
end
~
#40101
poison ninja~
0 n 100
~
dg_affect %self% poison on 48
~
#40102
ninja begs~
0 ab 5
~
say Please... help us!
cough
set roll %random.5%
if %roll% == 5
wait 1 sec
say Our villagers and children are sick... kids needs help with hurry...
wait 1 sec
say Please... go east and...
eval die %self.maxhitp% + 11
%damage% %self% %die%
end
~
#40103
sick quest~
0 g 100
~
if %actor.is_pc%
look %actor.name%
wait 1 sec
say Oh they found some help!
wait 2 sec
hug %actor.name%
wait 2 sec
say This poor child need his medicine but it is very expensive...
wait 2 sec
sad
wait 2 sec
if %actor.gold% <= 6000
say If you can give me 6000 coins I will be able to buy this medicine.
else
say If you can give me %actor.gold% coins I will be able to buy this medicine.
end
wait 2 sec
say or otherwise is poor child will lose his life...
wait 1 sec
cry child
end
~
#40104
donate medicine~
0 m 1
~
if %amount% >= 6000 && %actor.gold% /= 0
wait 1 sec
say Oh dear, you saved his life!
wait 2 sec
say We owe our lives to you...
wait 2 sec
%load% obj 40097 %self%
eval revert 40096
remote revert %self.inventory.id%
give card %actor.name%
%echo% A lot of energy bolts invade the house and attach with your new card.
%at% 40100 %echo% Something strange is happening here... you see some flash lights to the east.
%at% 40100 %purge% %self.room.people%
%at% 40100 %purge% %self.room.people%
%at% 40100 %purge% %self.room.people%
%at% 40100 %purge% %self.room.people%
%at% 40100 %purge% %self.room.people%
%at% 40100 %purge% %self.room.people%
%purge% child
%purge% %self%
else
say Thanks, but that is not enough...
give %amount% coins %actor.name%
end
~
#40105
load sick villagers quest~
2 f 10
~
%echo% This trigger commandlist is not complete!
~
#40196
old man ask to return~
0 n 100
~
wait 1 sec
set myroom %self.room%
if %myroom.vnum% != 40103
eval i %myroom.people%
while %i%
set next %i.next_in_room%
if %i.is_pc%
wait 1 sec
say Oh dear, you healed us!
wait 2 sec
say I see we are away from home...
wait 2 sec
say If not too much to ask, can you bring us back to home?
wait 1 sec
mfollow %i%
%send% %actor% The healthy villagers starts following you.
wait 1 sec
say Thank you, %i.name%. Let's go!
eval player %i.name%
remote player %self.id%
attach 40096 %self.id%
detach 40196 %self.id%
halt
break
end
set i %next%
done
else
attach 40096 %self.id%
detach 40196 %self.id%
halt
end
~
$~
