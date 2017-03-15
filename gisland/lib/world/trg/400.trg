#40000
player config~
2 q 100
~
if %direction% == down
%damage% %actor% -%actor.maxhitp%
if !%actor.eq(*)%
%load% obj 3202 %actor% rfinger
%load% obj 3203 %actor% inv
end
if !%actor.inventory(3203).contents%
eval book_purge_leaves 1
remote book_purge_leaves %actor.id%
%teleport% %actor% 1400
%load% obj 1001 %actor% inv
%load% obj 1002 %actor% inv
%load% obj 1003 %actor% inv
%load% obj 1004 %actor% inv
%load% obj 1005 %actor% inv
%load% obj 1006 %actor% inv
%load% obj 1007 %actor% inv
%load% obj 1008 %actor% inv
%load% obj 1009 %actor% inv
%load% obj 1010 %actor% inv
%load% obj 1011 %actor% inv
%load% obj 1013 %actor% inv
%load% obj 1014 %actor% inv
%load% obj 1015 %actor% inv
%load% obj 1016 %actor% inv
%load% obj 1017 %actor% inv
%load% obj 1018 %actor% inv
%load% obj 1019 %actor% inv
%load% obj 1020 %actor% inv
%load% obj 1021 %actor% inv
%load% obj 1024 %actor% inv
%load% obj 1025 %actor% inv
%load% obj 1026 %actor% inv
%load% obj 1027 %actor% inv
%load% obj 1030 %actor% inv
%load% obj 1031 %actor% inv
%load% obj 1032 %actor% inv
%load% obj 1033 %actor% inv
%load% obj 1034 %actor% inv
%load% obj 1037 %actor% inv
%load% obj 1039 %actor% inv
%load% obj 1040 %actor% inv
%load% obj 3119 %actor% inv
%load% obj 3120 %actor% inv
%load% obj 3130 %actor% inv
%load% obj 40000 %actor% inv
%load% obj 65303 %actor% inv
%force% %actor% open binder
%force% %actor% put all.card in binder
%force% %actor% close binder
rdelete book_purge_leaves %actor.id%
%force% %actor% cls
wait 0.1 sec
%force% %actor% down
else
%load% obj 1014 %actor% inv
end
end
~
#40001
suit of armor follow rat~
0 n 100
~
mfollow rat
~
#40002
remove armor if rat dies~
0 f 100
~
eval i %actor.room.people%
while %i%
set next %i.next_in_room%
if %i.vnum% == 40000
%echo% The suit of armor dismantles itself!  R.I.P.
eval xp 405 - (%actor.level% * 8)
%send% %actor% You receive %xp% experience points.
nop %actor.exp(%xp%)%
%purge% living
break
end
set i %next%
done
~
#40031
visited rabicuta tag~
2 q 100
~
if %actor.is_pc%
rdelete visited_rabicuta %actor.id%
eval visited_rabicuta %self.vnum%
remote visited_rabicuta %actor.id%
end
~
#40096
villagers reward player~
0 abi 100
~
set myroom %self.room%
if %myroom.vnum% == 40103
wait 1 sec
eval i %myroom.people%
while %i%
set next %i.next_in_room%
if %self.player% == %i.name%
say You healed us!
wait 2 sec
say We are eternally grateful to you...
wait 3 sec
say Wait here, I have something for you.
wait 2 sec
emote entered in the house.
wait 2 sec
emote left the house carrying a jewelry box.
wait 2 sec
say There, you deserve that.
wait 1 sec
%at% 40199 get card prizer
give card %self.player%
wait 1 sec
say Hope you like, now if you excuse us...
wait 1 sec
%echo% The villagers go to their homes.
%purge% villager
%purge% %self%
else
set ppl %random.char%
wait 1 sec
say You healed us!
wait 2 sec
say We are eternally grateful to you...
wait 3 sec
say Wait here, I have something for you.
wait 2 sec
emote entered in the house.
wait 2 sec
emote left the house carrying a jewelry box.
wait 2 sec
say There, you deserve that.
wait 1 sec
%load% obj 65375 %self% inv
give card %ppl.name%
wait 1 sec
say Hope you like, now if you excuse us...
wait 1 sec
%echo% The villagers go to their homes.
%purge% villager
%purge% %self%
end
else
if %random.10% == 10
say There is still a lot to get home?
end
~
#40097
sick quest fail~
1 fhj 100
~
set oldsdesc %self.shortdesc%
eval revert %self.vnum% + 100
if %self.worn_by%
%transform% %revert%
%send% %self.owner_id% The %oldsdesc% that you were holding transforms into a %self.shortdesc%... FOREVER!
%echoaround% %self.owner_id% The %oldsdesc% that %self.owner_name% were holding transforms into a %self.shortdesc%... FOREVER!
%load% obj %revert%
%purge% %self%
halt
end
if %self.is_inroom%
%transform% %revert%
%echo% The %oldsdesc% was lying on the ground turns into a %self.shortdesc%... FOREVER.
%load% obj %revert%
%purge% %self%
halt
end
eval owner_id %actor%
eval owner_name %actor.name%
remote owner_id %self.id%
remote owner_name %self.id%
otimer 2
~
#40099
visited rabicuta tag 2~
2 q 100
~
if %direction% == north && %actor.is_pc%
rdelete visited_rabicuta %actor.id%
eval visited_rabicuta %self.vnum%
remote visited_rabicuta %actor.id%
end
~
$~
