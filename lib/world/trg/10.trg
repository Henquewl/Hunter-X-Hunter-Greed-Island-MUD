#1000
eliminate spell~
1 c 1
gain~
if %self.carried_by.level% >= 31
if %cmd.mudcommand% == gain && %arg.is_pc%
if %arg.room% == %actor.room%
%force% %actor% say Eliminate ON! Target %arg.name%!
else
set prevloc %actor.room.vnum%
%force% %actor% say Eliminate ON! Target %arg.name%!
%teleport% %actor% %arg.room.vnum%
%force% %actor% say Eliminate ON! Target %arg.name%!
%teleport% %actor% %prevloc%
end
wait 2 sec
eval die %arg.maxhitp% + 20
%damage% %arg% %die%
else
%force% %actor% say Invalid target
halt
end
else
%force% %actor% gossip Not allowed gain %self.shortdesc%
%send% %actor% Only GMs are allowed to use this card.
%send% %actor% The card breaks and turn into dust.
%purge% %self%
end
~
#1001
peek spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Peek ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(castle_gate)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete castle_gate %arg.id%
%purge% %self%
halt
end
if %arg.varexists(blackout_curtain)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete blackout_curtain %arg.id%
%purge% %self%
halt
end
%send% %actor% You feels informed.
%send% %actor% %arg.name% Un-restricted cards:
eval counter 0
eval i %arg.inventory(3203).contents%
while %i%
set next %i.next_in_list%
if %i.type% != RESTRICTED
%send% %actor% %i.shortdesc%
eval counter %counter% +1
end
set i %next%
done
%send% %actor% Total: %counter%/45 cards.
%echoaround% %actor% %actor.name% feels informed.
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1002
fluroscopy spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Fluroscopy ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(castle_gate)%
%send% %actor% %arg.name% was protected by a spell!
%send% %arg% A defensive spell ends.
rdelete castle_gate %arg.id%
%purge% %self%
halt
end
if %arg.varexists(blackout_curtain)%
%send% %actor% %arg.name% was protected by a spell!
%send% %arg% A defensive spell ends.
rdelete blackout_curtain %arg.id%
%purge% %self%
halt
end
%send% %actor% You feels informed.
%send% %actor% %arg.name% Restricted cards:
eval counter 0
eval i %arg.inventory(3203).contents%
while %i%
set next %i.next_in_list%
if %i.type%==RESTRICTED
%send% %actor% %i.shortdesc%
eval counter %counter% +1
end
set i %next%
done
%send% %actor% Total: %counter%/100 cards.
%echoaround% %actor% %actor.name% feels informed.
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1003
defensive wall spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(defensive_wall)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Defensive Wall ON!
wait 1 sec
%force% %actor% remove %self.name%
detach 3216 %self.id%
attach 3226 %self.id%
%transform% 1053
eval defensive_wall %actor.name%
remote defensive_wall %actor.id%
%send% %actor% A faint aura covers you.
%echoaround% %actor% A faint aura covers %actor.name%.
wait 30 sec
%send% %actor% A faint aura that was protecting you vanishes.
%echoaround% %actor% A faint aura that was protecting %actor.name% vanishes.
rdelete defensive_wall %actor.id%
%purge% %self%
else
return 0
end
~
#1004
reflection spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(reflection_spell)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Reflection ON!
wait 1 sec
%force% %actor% remove %self.name%
detach 3216 %self.id%
attach 3226 %self.id%
%transform% 1054
eval reflection_spell 1
remote reflection_spell %actor.id%
%send% %actor% A faint aura covers you.
%echoaround% %actor% A faint aura covers %actor.name%.
wait 30 sec
%send% %actor% A faint aura that was protecting you vanishes.
%echoaround% %actor% A faint aura that was protecting %actor.name% vanishes.
rdelete defensive_wall %actor.id%
%purge% %self%
else
return 0
end
~
#1005
magnetic force spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Magnetic Force ON! Target %arg.name%!
%zoneecho% %arg.room.vnum% You see a bolt of energy flying through the skies.
wait 1 sec
%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.


set prevloc %self.room.vnum%
%teleport% %actor% %arg.room.vnum%
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1006
pick pocket spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Pick Pocket ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
if %arg.varexists(holy_water)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
eval holy_water %arg.holy_water% - 1
remote holy_water %arg.id%
if %arg.holy_water% < 1
rdelete holy_water %arg.id%
%send% %arg% A defensive spell ends.
%purge% %self%
halt
else
%send% %arg% A defensive spell weakens.
%purge% %self%
halt
end
end
eval max 0
eval u %arg.inventory(3203).contents%
while %u%
set next %u.next_in_list%
if %u.type% != RESTRICTED && %actor.varexists(good_luck)%
if %u.cost% >= 6600
%load% obj %u.vnum% %actor% inv
%send% %actor% You stealed %u.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% stealed a non-restricted card from you!
rdelete good_luck %actor.id%
%purge% %u%
break
end
end
if %u.type% != RESTRICTED
eval max %max% + 1
end
set u %next%
done
eval i %arg.inventory(3203).contents%
eval steal %%random.%max%%%
eval counter 0
while %i%
set next %i.next_in_list%
if %i.type% != RESTRICTED
eval counter %counter% + 1
if %steal% == %counter% 
break
end
end
set i %next%
done
if !%i%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%purge% %self%
halt
end
%load% obj %i.vnum% %actor% inv
%send% %actor% You stealed %i.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% stealed a non-restricted card from you!
%purge% %i%
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1007
thief spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Thief ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(prison_spell)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% Prison spell protects you.
%purge% %self%
halt
end
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
eval max 0
eval u %arg.inventory(3203).contents%
while %u%
set next %u.next_in_list%
if %u.type% == RESTRICTED && %actor.varexists(good_luck)%
if %u.cost% >= 130000
%load% obj %u.vnum% %actor% inv
%send% %actor% You stealed %u.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% stealed a restricted card from you!
rdelete good_luck %actor.id%
%purge% %u%
break
end
end
if %u.type% == RESTRICTED
eval max %max% + 1
end
set u %next%
done
eval i %arg.inventory(3203).contents%
eval steal %%random.%max%%%
eval counter 0
while %i%
set next %i.next_in_list%
if %i.type% == RESTRICTED
eval counter %counter% + 1
if %steal% == %counter% 
break
end
end
set i %next%
done
if !%i%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%purge% %self%
halt
end
%load% obj %i.vnum% %actor% inv
%send% %actor% You stealed %i.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% stealed a restricted card from you!
%purge% %i%
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1008
trade spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
* Check if actor not cheating.
eval c %actor.inventory%
while %c%
set next %c.next_in_list%
if %c.type% == SPELLCARD
%send% %actor% Trade spell only works when all the cards are in the binder.
halt
end
if %c.type% == UNRESTRICTED
%send% %actor% Trade spell only works when all the cards are in the binder.
halt
end
if %c.type% == RESTRICTED
%send% %actor% Trade spell only works when all the cards are in the binder.
halt
end
set c %next%
done
%force% %actor% say Trade ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
* Set actor max cards to random after
eval maxa 0
eval a %actor.inventory(3203).contents%
while %a%
set next %a.next_in_list%
eval maxa %maxa% + 1
set a %next%
done
* Set target max cards to random after
eval maxo 0
eval t %arg.inventory(3203).contents%
while %t%
set next %t.next_in_list%
eval maxo %maxo% + 1
set t %next%
done
* Select the trading card from actor
eval i %actor.inventory(3203).contents%
eval steala %%random.%maxa%%%
eval countera 0
while %i%
set next %i.next_in_list%
eval countera %countera% + 1
if %steala% == %countera% 
break
end
set i %next%
done
* Select the trading card from target
eval u %arg.inventory(3203).contents%
eval steal %%random.%maxo%%%
eval counter 0
while %u%
set next %u.next_in_list%
eval counter %counter% + 1
if %steal% == %counter% 
break
end
set u %next%
done
if !%i%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%purge% %self%
halt
end
if %u.type% == RESTRICTED && %arg.varexists(prison_spell)%
%send% %actor% %arg.name% was protected by a spell and trade fails!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% Prison spell protects you.
%purge% %self%
halt
end
if !%u%
%send% %actor% Your spell fails. Have no card in target binder to trade.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%purge% %self%
halt
end
%load% obj %i.vnum% %arg% inv
%load% obj %u.vnum% %actor% inv
%send% %actor% You traded your %i.shortdesc% for %u.shortdesc% with %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% exchange a card with you!
%purge% %i%
%purge% %u%
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1009
return spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg% == antokiba && %actor.varexists(visited_antokiba)%
%force% %actor% say Return ON! Antokiba!
wait 1 sec
%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.


set prevloc %self.room.vnum%
%teleport% %actor% %actor.visited_antokiba%
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
elseif %arg% == masadora && %actor.varexists(visited_masadora)%
%force% %actor% say Return ON! Masadora!
wait 1 sec
%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.


set prevloc %self.room.vnum%
%teleport% %actor% %actor.visited_masadora%
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
elseif %arg% == rabicuta && %actor.varexists(visited_rabicuta)%
%force% %actor% say Return ON! Rabicuta!
wait 1 sec
%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.


set prevloc %self.room.vnum%
%teleport% %actor% %actor.visited_rabicuta%
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
else
%send% %actor% This spell requires a target of previously visited city.
halt
end
else
return 0
end
~
#1010
transform spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
eval cardf %arg.car.name%
eval cards %arg.cdr.name%
if %cmd.mudcommand% == gain
* Check if you have the card will be transformed
eval f %actor.inventory%
while %f%
set next %f.next_in_list%
if %f.name% == %cardf%
if %f.type% == SPELLCARD
break
end
if %f.type% == UNRESTRICTED
break
end
if %f.type% == RESTRICTED
break
end	
end			
set f %next%
done
if !%f%
if %actor.inventory.type% == SPELLCARD
eval f %actor.inventory%
elseif %actor.inventory.type% == UNRESTRICTED
eval f %actor.inventory%
elseif %actor.inventory.type% == RESTRICTED
eval f %actor.inventory%
else
%send% %actor% You need at least 1 card into your inventory to be transformed.
halt
end
end
eval s %actor.inventory(3203).contents%
while %s%
set next %s.next_in_list%
if %s.name% == %cards%
if %s.type% == SPELLCARD
break
end
if %s.type% == UNRESTRICTED
break
end
if %s.type% == RESTRICTED
break
end
end
set s %next%
done
if !%s%
%send% %actor% The original card was not found, try a more specific name or number.
%send% %actor% Syntax: gain <base card name to be transformed> <orignal card name>
halt
end
%force% %actor% say Transform ON!
eval revert %f.vnum%
wait 1 sec
%load% obj %s.vnum% %actor% inv
remote revert %actor.inventory.id%
%send% %actor% The %f.shortdesc% transformed into %s.shortdesc%.
%echoaround% %actor% %actor.name% cast a spell card on himself.
%purge% %f%
%purge% %self%
else
return 0
end
~
#1011
clone spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Clone ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
eval max 0
eval u %arg.inventory(3203).contents%
while %u%
set next %u.next_in_list%
if %u.type% == RESTRICTED && %actor.varexists(good_luck)%
if %u.cost% >= 130000
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
eval revert %u.vnum%
remote revert %u.id%
%load% obj %u.vnum% %actor% inv 
%send% %actor% The %self.shortdesc% transformed into %actor.inventory.shortdesc%.
rdelete good_luck %actor.id%
%purge% %self%
break
end
end
if %u.type% == RESTRICTED
eval max %max% + 1
end
set u %next%
done
eval i %arg.inventory(3203).contents%
eval steal %%random.%max%%%
eval counter 0
while %i%
set next %i.next_in_list%
if %i.type% == RESTRICTED
eval counter %counter% + 1
if %steal% == %counter% 
break
end
end
set i %next%
done
if !%i%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%purge% %self%
halt
end
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
eval revert %i.vnum%
remote revert %i.id%
%load% obj %i.vnum% %actor% inv 
%send% %actor% The %self.shortdesc% transformed into %actor.inventory.shortdesc%.
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1012
railguide spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Railguide ON! Target %arg.name%!
wait 1 sec
%force% %arg% close binder
%force% %actor% recite railguide %arg.name%
halt
elseif !%arg%
%force% %actor% say Railguide ON!
wait 1 sec
%force% %arg% close binder
%force% %actor% recite railguide me
halt
else
%send% %actor% Cannot find the target of your spell!
halt
end
else
return 0
end
~
#1013
departure effect~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
eval depart 40000 + %random.8%
%force% %actor% say Departure ON!
%zoneecho% %depart% You see a bolt of energy flying through the skies.
wait 1 sec

%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.

set prevloc %self.room.vnum%
%teleport% %actor% %depart%
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
else
return 0
end
~
#1014
leave spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Leave ON!
wait 1 sec

%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.

set prevloc %self.room.vnum%
%teleport% %actor% 1406
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
else
return 0
end
~
#1015
sightvision spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Sightvision ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(castle_gate)%
%send% %actor% %arg.name% was protected by a spell!
%send% %arg% A defensive spell ends.
rdelete castle_gate %arg.id%
%purge% %self%
halt
end
if %arg.varexists(blackout_curtain)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete blackout_curtain %arg.id%
%purge% %self%
halt
end
%send% %actor% You feels informed.
%send% %actor% %arg.name% cards:
eval countert 0
eval counterr 0
eval counters 0
eval counteru 0
eval i %arg.inventory(3203).contents%
while %i%
set next %i.next_in_list%
if %i.type% == RESTRICTED
%send% %actor% %i.shortdesc%
eval countert %countert% +1
eval counterr %counterr% +1
end
if %i.type% == SPELLCARD
%send% %actor% %i.shortdesc%
eval countert %countert% +1
eval counters %counters% +1
end
if %i.type% == UNRESTRICTED
%send% %actor% %i.shortdesc%
eval countert %countert% +1
eval counteru %counteru% +1
end
set i %next%
done
eval untotal %counters% + %counteru%
%send% %actor% Restricted:     %counterr%/100 cards.
%send% %actor% Spell:          %counters%/45 cards.
%send% %actor% Un-restricted:  %counteru%/45 cards.
%send% %actor% Non-restricted: %untotal%/45 cards.
%send% %actor% Total:          %countert%/145 cards.
%echoaround% %actor% %actor.name% feels informed.
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1016
drift spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Drift ON!
wait 1 sec
if  %actor.varexists(visited_antokiba)% && %actor.varexists(visited_masadora)% && %actor.varexists(visited_rabicuta)%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%purge% %self%
halt
end
eval i %random.3%
while %i%
set next %i%
if %i% == 1 && !%actor.varexists(visited_antokiba)%
%zoneecho% 12064 You see a bolt of energy flying through the skies.
eval visited_antokiba 12064
remote visited_antokiba %actor.id%
%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.


set prevloc %self.room.vnum%
%teleport% %actor% 12064
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
break
halt
end
if %i% == 2 && !%actor.varexists(visited_masadora)%
%zoneecho% 3053 You see a bolt of energy flying through the skies.
eval visited_masadora 3053
remote visited_masadora %actor.id%
%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.


set prevloc %self.room.vnum%
%teleport% %actor% 3053
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
break
halt
end
if %i% == 3 && !%actor.varexists(visited_rabicuta)%
%zoneecho% 40031 You see a bolt of energy flying through the skies.
eval visited_rabicuta 40031
remote visited_rabicuta %actor.id%
%send% %actor% A bolt of energy envelops you!
%echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.


set prevloc %self.room.vnum%
%teleport% %actor% 40031
%zoneecho% %prevloc% You see a bolt of energy flying through the skies.
%echoaround% %actor% A bolt of energy hits the ground near at you!
wait 0.1 sec
%send% %actor% You landed somewhere.
%force% %actor% look
%purge% %self%
break
halt
end
set i %random.3%
done
else
return 0
end
~
#1017
collision spell~
1 c 1
ga~
if %self.carried_by% == %actor%
  %echo% You must hold something before gain it.
  halt
end
if !%actor.is_book%
  %send% %actor% You need your book activated before gain a spell card.
  halt
end
if %cmd.mudcommand% == gain
  %force% %actor% say Collision ON!
  while (%counter% < 100)
    makeuid mob 2+%counter%
    if %mob.is_pc% == 1
      if %mob.id% != %actor.id%
        if !%mob.varexists(met_%actor.name%)%
          %zoneecho% %mob.room.vnum% You see a bolt of energy flying through the skies.
          wait 1 sec
          %send% %actor% A bolt of energy envelops you!
          %echoaround% %actor% A bolt of energy envelops %actor.name% and sent %actor.himher% to far away.          
          set prevloc %self.room.vnum%
          %teleport% %actor% %mob.room.vnum%
          %zoneecho% %prevloc% You see a bolt of energy flying through the skies.
          %echoaround% %actor% A bolt of energy hits the ground near at you!
          wait 0.1 sec
          eval met_%actor.name% 1
          eval met_%mob.name% 1
          remote met_%actor.name% %mob.id%
          remote met_%mob.name% %actor.id%
          %send% %actor% You landed somewhere.
          %force% %actor% look
          %purge% %self%
        end
      end
    end
    eval counter %counter% + 1
  done
  %send% %actor% The %self.shortdesc% spell fails and dissolves.
  %purge% %self%
end
~
#1018
levy spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Levy ON!
if %actor.room.roomflag(NO_MAGIC)%
%send% %actor% Nothing happens.
halt
end
eval i %actor.room.people%
while %i%
set next %i.next_in_room%
if %i.is_pc% == 1
if %i% != %self.worn_by%
eval max 0
eval u %i.inventory(3203).contents%
while %u%
set next %u.next_in_list%
if %actor.varexists(good_luck)% && !%i.varexists(prison_spell)%
if %u.type% == RESTRICTED
%load% obj %u.vnum% %actor% inv
%send% %actor% You stealed %u.shortdesc% from %i.name%!
%echoaround% %actor% %actor.name% cast a spell card on %i.name%.
%send% %i% %actor.name% stealed a card from you!
%purge% %u%
break
end
end
eval max %max% + 1
set u %next%
done
if !%actor.varexists(good_luck)%
eval y %i.inventory(3203).contents%
eval steal %%random.%max%%%
eval counter 0
while %y%
set next %y.next_in_list%
eval counter %counter% + 1
if %steal% == %counter% 
break
end
set y %next%
done
if %i.varexists(reflection_spell)%
set card %actor.inventory(3203).contents%
%send% %actor% %i.name% was protected by a reflective spell!
%load% obj %card.vnum% %i% inv
%send% %i% You stealed %card.shortdesc% from %actor.name%!
%purge% %card%
%echoaround% %i% %i.name% reflects a spell card back on %actor.name%.
%send% %i% A defensive spell ends.
rdelete reflection_spell %i.id%
%purge% %i.inventory(1054)%
set y 0
elseif %i.varexists(defensive_wall)%
%send% %actor% %i.name% was protected by a spell!
%send% %i% A defensive spell ends.
rdelete defensive_wall %i.id%
%purge% %i.inventory(1053)%
set y 0
elseif %i.varexists(castle_gate)%
%send% %actor% %i.name% was protected by a spell!
%send% %i% A defensive spell ends.
rdelete castle_gate %i.id%
%purge% %i.inventory(1069)%
set y 0
elseif %i.varexists(holy_water)%
%send% %actor% %i.name% was protected by a spell!
eval holy_water %i.holy_water% - 1
remote holy_water %i.id%
if %i.holy_water% < 1
rdelete holy_water %i.id%
%send% %i% A defensive spell ends.
%purge% %i.inventory(1076)%
set y 0
else
%send% %i% A defensive spell weakens.
set y 0
end
end
if !%y%
%send% %actor% You stealed nothing from %i.name%.
%echoaround% %actor% %actor.name% cast a spell card on %i.name%.
else
%load% obj %y.vnum% %actor% inv
%send% %actor% You stealed %y.shortdesc% from %i.name%!
%echoaround% %actor% %actor.name% cast a spell card on %i.name%.
%send% %i% %actor.name% stealed a card from you!
%purge% %y%
end
end
end
end
set i %next%
done
rdelete good_luck %actor.id%
wait 2 sec
%purge% %self%
else
return 0
end
~
#1019
castle wall spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(castle_gate)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Castle Wall ON!
wait 1 sec
%force% %actor% remove %self.name%
detach 3216 %self.id%
attach 3226 %self.id%
%transform% 1069
eval castle_gate %actor.name%
remote castle_gate %actor.id%
%send% %actor% A faint aura covers you.
%echoaround% %actor% A faint aura covers %actor.name%.
wait 30 sec
%send% %actor% A faint aura that was protecting you vanishes.
%echoaround% %actor% A faint aura that was protecting %actor.name% vanishes.
rdelete castle_gate %actor.id%
%purge% %self%
else
return 0
end
~
#1020
fake spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Fake ON!
wait 1 sec
eval revert %self.vnum%
eval card 65300 + %random.17%
%load% obj %card% %actor% inv
set new %actor.inventory%
%send% %actor% The %self.shortdesc% transformed into %new.shortdesc%.
%echoaround% %actor% %actor.name% cast a spell card on himself.
detach all %new.id%
remote revert %new.id%
%purge% %self%
else
return 0
end
~
#1021
rob spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
eval card %arg.car%
eval target %arg.cdr%
if %cmd.mudcommand% == gain
if %target.is_pc%
%force% %actor% say Rob ON! Target %target.name%!
wait 1 sec
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
eval s %target.inventory(3203).contents%
while %s%
set next %s.next_in_list%
if %s.name% == %card.name%
if %s.type% == SPELLCARD
break
end
if %s.type% == UNRESTRICTED
break
end
if %s.type% == RESTRICTED
if %arg.varexists(prison_spell)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% Prison spell protects you.
%purge% %self%
halt
end
break
end
end
set s %next%
done
if !%s%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%echoaround% %actor% %actor.name% cast a spell card on %target.name%.
%purge% %self%
halt
end
%load% obj %s.vnum% %actor% inv
%send% %actor% You stealed %s.shortdesc% from %target.name%!
%echoaround% %actor% %actor.name% cast a spell card on %target.name%.
%send% %target% %actor.name% stealed a card from your binder!
%purge% %s%
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1024
penetrate spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Penetrate ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(castle_gate)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
eval s %arg.inventory(3203).contents%
while %s%
set next %s.next_in_list%
if %s.revert% > 0
if %s.type% == SPELLCARD
%load% obj %s.revert% %arg% inv
%purge% %s%
end
if %s.type% == UNRESTRICTED
%load% obj %s.revert% %arg% inv
%purge% %s%
end
if %s.type% == RESTRICTED
if !%arg.varexists(prison_spell)%
%purge% %s%
end
end
end
set s %next%
done
%send% %actor% You reverted all cards from %arg.name%'s binder.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% reverted all cards inside your binder!
%purge% %self%
elseif !%arg%
%force% %actor% say Penetrate ON!
wait 1 sec
eval i %actor.inventory(3203).contents%
while %i%
set next %i.next_in_list%
if %i.revert% > 0
if %i.type% == SPELLCARD
%load% obj %i.revert% %actor% inv
%purge% %i%
end
if %i.type% == UNRESTRICTED
%load% obj %i.revert% %actor% inv
%purge% %i%
end
if %i.type% == RESTRICTED
%purge% %i%
end
end
set i %next%
done
%send% %actor% You reverted all cards from your binder.
%echoaround% %actor% %actor.name% cast a spell card on %actor.himher% self.
%purge% %self%
else
%send% %actor% Cannot find the target of your spell!
halt
end
else
return 0
end
~
#1025
blackout curtain spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(blackout_curtain)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Blackout Curtain ON!
wait 1 sec
eval blackout_curtain %actor.name%
remote blackout_curtain %actor.id%
%send% %actor% A strong aura covers you.
%echoaround% %actor% A strong aura covers %actor.name%.
else
return 0
end
~
#1026
holy water spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(holy_water)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Holy Water ON!
wait 1 sec
%force% %actor% remove %self.name%
detach 3216 %self.id%
attach 3226 %self.id%
%transform% 1076
eval holy_water 10
remote holy_water %actor.id%
%send% %actor% A strong aura covers you.
%echoaround% %actor% A strong aura covers %actor.name%.
else
return 0
end
~
#1027
trace spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Trace ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
if %arg.varexists(holy_water)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
eval holy_water %arg.holy_water% - 1
remote holy_water %arg.id%
if %arg.holy_water% < 1
rdelete holy_water %arg.id%
%send% %arg% A defensive spell ends.
%purge% %arg.inventory(1076)%
%purge% %self%
halt
else
%send% %arg% A defensive spell weakens.
%purge% %self%
halt
end
end
eval trace_%arg.name% 1
remote trace_%arg.name% %arg.id%
remote trace_%arg.name% %actor.id%
%send% %actor% Now you can trace %arg.name%.
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1028
stone throw spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Stone Throw ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
if %arg.varexists(holy_water)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
eval holy_water %arg.holy_water% - 1
remote holy_water %arg.id%
if %arg.holy_water% < 1
rdelete holy_water %arg.id%
%send% %arg% A defensive spell ends.
%purge% %arg.inventory(1076)%
%purge% %self%
halt
else
%send% %arg% A defensive spell weakens.
%purge% %self%
halt
end
end
eval max 0
eval u %arg.inventory(3203).contents%
while %u%
set next %u.next_in_list%
if %u.type% != RESTRICTED && %actor.varexists(good_luck)%
if %u.cost% >= 6600
%load% obj %u.vnum% %actor% inv
%send% %actor% You detroyed %u.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% destroyed a non-restricted card from you!
rdelete good_luck %actor.id%
%purge% %u%
break
end
end
if %u.type% != RESTRICTED
eval max %max% + 1
end
set u %next%
done
eval i %arg.inventory(3203).contents%
eval steal %%random.%max%%%
eval counter 0
while %i%
set next %i.next_in_list%
if %i.type% != RESTRICTED
eval counter %counter% + 1
if %steal% == %counter% 
break
end
end
set i %next%
done
if !%i%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%purge% %self%
halt
end
%send% %actor% You destroyed %i.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% destroyed a non-restricted card from you!
%purge% %i%
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1029
shot spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Shot ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(prison_spell)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% Prison spell protects you.
%purge% %self%
halt
end
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
eval max 0
eval u %arg.inventory(3203).contents%
while %u%
set next %u.next_in_list%
if %u.type% == RESTRICTED && %actor.varexists(good_luck)%
if %u.cost% >= 130000
%send% %actor% You destroyed %u.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% destroyed a restricted card from you!
rdelete good_luck %actor.id%
%purge% %u%
break
end
end
if %u.type% == RESTRICTED
eval max %max% + 1
end
set u %next%
done
eval i %arg.inventory(3203).contents%
eval steal %%random.%max%%%
eval counter 0
while %i%
set next %i.next_in_list%
if %i.type% == RESTRICTED
eval counter %counter% + 1
if %steal% == %counter% 
break
end
end
set i %next%
done
if !%i%
%send% %actor% The %self.shortdesc% spell fails and dissolves.
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%purge% %self%
halt
end
%send% %actor% You destroyed %i.shortdesc% from %arg.name%!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% %actor.name% destroyed a restricted card from you!
%purge% %i%
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1030
guidepost spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
set number %arg%
if %cmd.mudcommand% == gain
if %number% > 0 && %number% < 100
%force% %actor% say Guidepost ON! Number %number%
wait 1 sec
* %echoaround% %actor% %actor.name% feels informed.
switch %number%
case 75
set item 653%number%
%load% obj %item% %actor% inv
%send% %actor% %actor.inventory.shortdesc% is a random quest when walking by A Woody Plateau.
%purge% %actor.inventory%
%purge% %self%
break
case 84
set item 653%number%
%load% obj %item% %actor% inv
%send% %actor% %actor.inventory.shortdesc% is in the lower level of the newbie zone.
%purge% %actor.inventory%
%purge% %self%
break
default
%send% %actor% This card number does not exist yet, try anUNRESTRICTED.
break
done
else
%send% %actor% This spell requires a number between 1 and 99.
halt
end
else
%send% %actor% This spell requires a number between 1 and 99.
end
~
#1031
analysis spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
set number %arg%
if %cmd.mudcommand% == gain
if %number% > 0 && %number% < 100
%force% %actor% say Analysis ON! Number %number%
wait 1 sec
* %echoaround% %actor% %actor.name% feels informed.
switch %number%
case 75
set item 653%number%
%load% obj %item% %actor% inv
%send% %actor% %actor.inventory.shortdesc% has no practical use.
%purge% %actor.inventory%
%purge% %self%
break
case 84
set item 653%number%
%load% obj %item% %actor% inv
%send% %actor% %actor.inventory.shortdesc% while worn protects from curses and restore affected held cards.
%purge% %actor.inventory%
%purge% %self%
break
default
%send% %actor% This card number does not exist yet, try anUNRESTRICTED.
break
done
else
%send% %actor% This spell requires a number between 1 and 99.
halt
end
else
%send% %actor% This spell requires a number between 1 and 99.
end
~
#1032
lottery spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Lottery ON!
wait 1 sec
set old %self.shortdesc%
eval card %random.100%
if %actor.varexists(good_luck)%
set card 100
rdelete good_luck %actor.id%
end
switch %card%
case 91
case 92
case 93
case 94
case 95
case 96
case 97
case 98
case 99
eval i %actor.inventory%
while %i%
eval spell 1000 + %random.40%
%load% obj %spell% %actor% inv
if %actor.inventory.vnum% == %spell%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% Your %old% transforms into %new%.
%purge% %self%
halt
break
case 100
eval i %actor.inventory%
while %i%
eval unusual 65300 + %random.99%
%load% obj %unusual% %actor% inv
if %actor.inventory.vnum% == %unusual%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% Your %old% transforms into %new%.
%purge% %self%
halt
break
default
switch %random.2%
case 1
eval i %actor.inventory%
while %i%
eval free 3119 + %random.70%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% Your %old% transforms into %new%.
%purge% %self%
halt
break
case 2
eval i %actor.inventory%
while %i%
eval free 39999 + %random.100%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% Your %old% transforms into %new%.
%purge% %self%
halt
break
default
break
done
done
else
return 0
end
~
#1033
adhesion spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Adhesion ON! Target %arg.name%!
wait 1 sec
if %arg.varexists(defensive_wall)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
%send% %arg% A defensive spell ends.
rdelete defensive_wall %arg.id%
%purge% %self%
halt
end
if %arg.varexists(holy_water)%
%send% %actor% %arg.name% was protected by a spell!
%echoaround% %actor% %actor.name% cast a spell card on %arg.name%.
eval holy_water %arg.holy_water% - 1
remote holy_water %arg.id%
if %arg.holy_water% < 1
rdelete holy_water %arg.id%
%send% %arg% A defensive spell ends.
%purge% %arg.inventory(1076)%
%purge% %self%
halt
else
%send% %arg% A defensive spell weakens.
%purge% %self%
halt
end
end
eval adhesion_%arg.name% 1
remote adhesion_%arg.name% %arg.id%
remote adhesion_%arg.name% %actor.id%
%send% %actor% Now you can use adhesion command to check %arg.name% restricted cards.
%purge% %self%
else
%send% %actor% This spell requires a player target.
halt
end
else
return 0
end
~
#1034
purify spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
eval f %actor.inventory%
while %f%
set next %f.next_in_list%
if %f.name% == %arg.name%
if %f.weight% <= 1
break
end
end
set f %next%
done
if !%f%
%send% %actor% You do not have such card.
halt
end
%force% %actor% say Purify ON!
wait 1 sec
if !%f.revert%
%send% %actor% Nothing happens and your spell card dissolves.
%purge% %self%
end
%load% obj %f.revert% %actor% inv
%send% %actor% You transforms back your %f.shortdesc% into %actor.inventory.shortdesc%.
%echoaround% %actor% %actor.name% restored a card in %actor.hisher% possession.
%purge% %f%
%purge% %self%
halt
else
return 0
end
~
#1035
prison spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(prison_spell)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say Prison ON!
wait 1 sec
eval prison_spell 1
remote prison_spell %actor.id%
%send% %actor% A strong aura covers you.
%echoaround% %actor% A strong aura covers %actor.name%.
%purge% %self%
else
return 0
end
~
#1036
god eye spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(god_eye)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
%force% %actor% say God Eye ON!
wait 1 sec
eval god_eye 1
remote god_eye %actor.id%
%purge% %self%
else
return 0
end
~
#1037
recycle spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
eval f %actor.inventory%
while %f%
set next %f.next_in_list%
if %f.name% == %arg.name%
if %f.weight% >= 2
break
end
end
set f %next%
done
if !%f%
%send% %actor% You do not have such item.
halt
end
%force% %actor% say Recycle ON!
wait 1 sec
%load% obj %f.vnum% %actor% inv
%send% %actor% You restored your %f.shortdesc%.
%echoaround% %actor% %actor.name% restored an item in %actor.hisher% possession.
%purge% %f%
%purge% %self%
halt
else
return 0
end
~
#1038
list spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%send% %actor% Cannot find the target of your spell!
halt
elseif %arg.is_pc% == 0
%send% %actor% Cannot find the target of your spell!
halt
elseif !%arg%
%send% %actor% Cannot find the target of your spell!
halt
else
eval locate %arg%
remote locate %actor.id%
attach 1088 %actor.id%
%force% %actor% say List ON! %arg%!
end
~
#1039
accompany spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
set target %arg.room.vnum%
%force% %actor% say Accompany ON! %arg.name%!
%zoneecho% %target% You see a bolt of energy flying through the skies.
wait 1 sec
* Your master first
%send% %actor.master% A bolt of energy envelops you!
%teleport% %actor.master% %target%
%force% %actor.master% look
* Now your followers
eval i %actor.room.people%
while %i%
set next %i.next_in_room%
if %i.master% == %actor%
%send% %i% A bolt of energy envelops you!

%teleport% %i% %target%
%force% %i% look
end
set i %next%
done
* And now you
%echoaround% %actor% A bolt of energy envelops %actor.name%'s group and sent them far away.
%send% %target% A bolt of energy hits the ground near at you!
%echoaround% %target% A bolt of energy hits the ground near at you!


%teleport% %actor% %target%
%force% %actor% look
%purge% %self%
else
if %arg% == masadora && %actor.varexists(visited_masadora)%
set target %actor.visited_masadora%
%force% %actor% say Accompany ON! Masadora!
%zoneecho% %target% You see a bolt of energy flying through the skies.
wait 1 sec
* Your master first
%send% %actor.master% A bolt of energy envelops you!

%teleport% %actor.master% %target%
%force% %actor.master% look
* Now your followers
eval i %actor.room.people%
while %i%
set next %i.next_in_room%
if %i.master% == %actor%
%send% %i% A bolt of energy envelops you!

%teleport% %i% %target%
%force% %i% look
end
set i %next%
done
* And now you
%echoaround% %actor% A bolt of energy envelops %actor.name%'s group and sent them far away.


%teleport% %actor% %target%
%echoaround% %actor% A bolt of energy hits the ground near at you!
%force% %actor% look
%purge% %self%
halt
elseif %arg% == antokiba && %actor.varexists(visited_antokiba)%
set target %actor.visited_antokiba%
%force% %actor% say Accompany ON! Antokiba!
%zoneecho% %target% You see a bolt of energy flying through the skies.
wait 1 sec
* Your master first
%send% %actor.master% A bolt of energy envelops you!

%teleport% %actor.master% %target%
%force% %actor.master% look
* Now your followers
eval i %actor.room.people%
while %i%
set next %i.next_in_room%
if %i.master% == %actor%
%send% %i% A bolt of energy envelops you!

%teleport% %i% %target%
%force% %i% look
end
set i %next%
done
* And now you
%echoaround% %actor% A bolt of energy envelops %actor.name%'s group and sent them far away.


%teleport% %actor% %target%
%echoaround% %actor% A bolt of energy hits the ground near at you!
%force% %actor% look
%purge% %self%
halt
elseif %arg% == rabicuta && %actor.varexists(visited_rabicuta)%
set target %actor.visited_rabicuta%
%force% %actor% say Accompany ON! Rabicuta!
%zoneecho% %target% You see a bolt of energy flying through the skies.
wait 1 sec
* Your master first
%send% %actor.master% A bolt of energy envelops you!

%teleport% %actor.master% %target%
%force% %actor.master% look
* Now your followers
eval i %actor.room.people%
while %i%
set next %i.next_in_room%
if %i.master% == %actor%
%send% %i% A bolt of energy envelops you!

%teleport% %i% %target%
%force% %i% look
end
set i %next%
done
* And now you
%echoaround% %actor% A bolt of energy envelops %actor.name%'s group and sent them far away.


%teleport% %actor% %target%
%echoaround% %actor% A bolt of energy hits the ground near at you!
%force% %actor% look
%purge% %self%
halt
end
%send% %actor% Nothing around by that name.
halt
end
else
return 0
end
~
#1040
contact spell~
1 c 1
ga~

if !%actor.is_book%
%send% %actor% You need your book activated before gain a spell card.
halt
end
if %actor.varexists(contact)%
%send% %actor% This spell is already active.
halt
end
if %cmd.mudcommand% == gain
if %arg.is_pc% == 1
%force% %actor% say Contact ON! %arg.name%!
%at% 1407 %force% elena tell %arg.name% A player used a Contact to start a communication with you. Just use 	Rreply	r.
%force% %actor% remove card
%transform% 1080
wait 1 sec
eval contact %arg.name%
remote contact %actor.id%
eval contact %actor.name%
remote contact %arg.id%
%at% 1407 %force% elena tell %actor.name% Now you can 	Rtell	r with the UNRESTRICTED player for the next 3 minutes.
wait 180 sec
%send% %arg% Contact with the player ends.
%send% %actor% the communication time with the player got expired.
rdelete contact %actor.id%
rdelete contact %arg.id%
%purge% %self%
halt
else
%send% %actor% Cannot find the target of your spell!
halt
else
return 0
end
~
#1070
purge card~
1 f 100
~
if %self.worn_by%
%send% %actor% The card breaks and turn into dust.
%purge% %self%
halt
end
if %self.is_inroom%
%send% %actor% The card breaks and turn into dust.
%purge% %self%
halt
end
otimer 2
~
#1088
locate object~
0 d 0
List ON!~
set lvl %self.level%
wait 1 sec
nop %actor.level(30)%
dg_cast 'locate obj' %self.locate%
nop %actor.level(%lvl%)%
rdelete locate %self.id%
%purge% %self.eq(hold)%
detach 1088 %self.id%
~
$~
