#40000
Login player~
2 s 100
~
wait 0.1 sec
if ((%actor.level% > 1) || (%actor.inventory(3203)%))
  if !%actor.eq(*)%
    %load% obj 3202 %actor% rfinger
  end
  if !%actor.inventory(3203)%
    %load% obj 3203 %actor% inv
  end
  %force% %actor% receive
  %force% %actor% down
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
%echo% A living armor collapses!  R.I.P.
set xp 31
nop %actor.exp(%xp%)%
%purge% living
break
end
set i %next%
done
~
#40003
villagers thank the player~
0 n 100
~
wait 2 sec
say what? we are healthy again?
wait 2 sec
say Thank you for saving our lives!
wait 2 sec
smile
wait 1 sec
%echo% %self.name% go back way to home.
%purge% %self%
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
gain healthy villagers~
1 c 1
ga~
if %cmd.mudcommand% == gain
  %force% %actor% say GAIN!
  wait 2 sec
  %load% mob 40196
  %echo% %self.shortdesc% turns into a bunch of healthy villagers.
  %load% obj 40196 %actor%
  %purge% %self%
else
  return 0
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
