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
Old man reward~
0 n 100
~
wait 1 sec
eval player %self.room.people%
while %player%
  set next %player.next_in_room%
  if %player.is_pc%
    say Oh dear, you saved us!
    wait 1 sec
    say Take this as a token of our gratitude, was with my family for generations
    wait 1 sec
    %load% obj 65475 %self% inv
    give ring %player.name%
    wait 1 sec
    say Farewell, adventurer
    %purge% %self%
	halt
  end
  set player %next%
done
%load% obj 65375
%purge% %self%
~
#40197
Villagers quest completed~
1 b 100
~
%echo% The old man comes out of the crowd.
%load% mob 40196
%purge% %self%
~
#40199
No purge timer~
1 t 100
~
* No Script
~
$~
