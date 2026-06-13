#65400
Keychain takes you to Housing~
1 j 1
~
* Thanks to bakarus for suggesting the return, to Vatiken
*  for helping him with it and to Rumble for adding it to
*  trigger 176 where I could copy it.
*  http://www.tbamud.com/forum/3-building/355-dg-script-question#371
wait 1 sec
* Adjust zone to proper zone number
set zone 654
set roomvnummin %zone%00
set roomvnummax %zone%99
* if person hasn't used the key before, send to Midgaard Temple
*   instead of returning.
set defaultroom 3001
* if person uses key in apartment zone, return player to last room
*   out of the zone where the key was used.
if %actor.room.vnum% >= %zone%00 && %actor.room.vnum% <= %zone%99
  if %actor.varexists(keychain_return_room)%
    %send% %actor% You return to your previous location.
    %echoaround% %actor% %actor.name% heads back out into the world.
    %teleport% %actor% %actor.keychain_return_room%
    %force% %actor% look
    %echoaround% %actor% %actor.name% appears in the room.
  else
    %send% %actor% You head back out into the world.
    %echoaround% %actor% %actor.name% heads back out into the world.
    %teleport% %actor% %defaultroom%
    %force% %actor% look
    %echoaround% %actor% %actor.name% appears in the room.
  end
else
  eval keychain_return_room %actor.room.vnum%
  remote  keychain_return_room %actor.id%
  %send% %actor% You head for home.
  %echoaround% %actor% %actor.name% heads for home.
  %teleport% %actor% %self.vnum%
  %force% %actor% look
  %echoaround% %actor% %actor.name% appears, heading for home.
end
%force% %actor% remove keychain
~
#65401
no recite~
1 c 2
rec~
if %cmd.mudcommand% == recite && %self.name% /= %arg%
%send% %actor% You can only recite scrolls.
else
return 0
end
~
#65402
pitcher refill~
1 ab 100
~
%transform% 1402
set %obj.weight% == 0
~
#65404
falling~
2 ab 100
~
set actor %random.char%
wait 1 sec
%force% %actor% down
~
#65417
archangel~
0 n 100
~
wait 5 sec
say 	CTell me, who you want me to heal?	n
attach 3249 %self.id%
~
#65419
cane of healing tap~
1 c 3
tap~
if %actor.hit% >= %actor.maxhit%
%send% %actor% You have no wounds left to mend.
else
%send% %actor% You tap the cane against your wounds and they knit closed.
%echoaround% %actor% %actor.name% taps a cane against %actor.hisher% wounds, which close before your eyes.
%damage% %actor% -50
end
~
#65420
explosive marbles throw~
1 c 2
throw~
if %self.name% /= %arg%
%send% %actor% You hurl the explosive marbles to the ground; they detonate!
%echoaround% %actor% %actor.name% hurls a handful of marbles that explode violently!
set p %actor.room.people%
while %p%
set nextp %p.next_in_room%
if %p% != %actor%
%damage% %p% 75
end
set p %nextp%
done
%purge% %self%
else
return 0
end
~
#65425
risky dice~
1 h 100
~
attach 65325 %self.id%
return 1
wait 0.1 sec
%send% %actor% The dice is rolling...
wait 2 sec
switch %random.20%
case 20
%send% %actor% The dice landed with "Skull" face up!
wait 1 sec
if %actor.room.roomflag(INDOORS)%
%send% %actor% A piece of the ceiling collapses on top of your head.
%echoaround% %actor% A piece of the ceiling collapses on top of %actor.name%'s head.
eval die %actor.maxhitp% + 32
%damage% %actor% %die%
detach 65325 %self.id%
halt
end
%send% %actor% The floor collapses and you fall into a kind of thorns trap that was hidden.
%echoaround% %actor% The floor collapses and %actor.name% fall into a kind of thorns trap that was hidden.
eval die %actor.maxhitp% + 20
%damage% %actor% %die%
detach 65325 %self.id%
halt
default
%send% %actor% The dice landed with "Good Luck" face up!
eval good_luck %actor%
remote good_luck %actor.id%
detach 65325 %self.id%
halt
done
~
#65426
Restricted 26 gain~
0 f 5
~
%load% obj 65326 %self% inv
~
#65446
gold dust girl quest~
0 g 100
~
wait 1 sec
stand
wait 1 sec
emote looks at you.
wait 1 sec
scream
say YAAAAAAAAAAAAAAAAAAAAIHHHHHHHHHHHHHH!
wait 1 sec
%load% obj 65346 %self% inv
if %self.has_item(65346)%
%echo% %self.name% transforms into %self.inventory(65346).shortdesc% that falls to the ground.
%purge% %self%
else
%echo% %self.name% vanishes trying to transform into a card but it reached the limit of transformations.
%purge% %self%
end
~
#65448
fairys breath inhale~
1 c 3
inhale~
%send% %actor% You inhale the fairy's breath and your body turns weightless!
%echoaround% %actor% %actor.name% inhales from a tiny vial and lifts gently off the ground.
dg_affect %actor% FLY on 60
%purge% %self%
~
#65451
flame guitar play~
1 c 3
play~
if %self.name% /= %arg%
%send% %actor% You strum the Flame Guitar and sheets of fire roar outward!
%echoaround% %actor% %actor.name% strums a guitar and sheets of fire roar across the room!
set p %actor.room.people%
while %p%
set nextp %p.next_in_room%
if %p% != %actor%
%damage% %p% 60
end
set p %nextp%
done
else
return 0
end
~
#65455
sleep clock ring~
1 c 3
ring~
if %self.name% /= %arg%
%send% %actor% You ring the clock and a drowsy chime fills the air.
%echoaround% %actor% %actor.name% rings a clock and a heavy, drowsy chime fills the air.
set p %actor.room.people%
while %p%
set nextp %p.next_in_room%
if %p% != %actor%
dg_affect %p% SLEEP on 15
end
set p %nextp%
done
else
return 0
end
~
#65461
Scanner~
1 c 4
scan~
if %actor.gold% >= 500
  %send% %actor% You lay down on the scanner table and it starts.
  %echoaround% %actor% %actor.name% lay down on the scanner table and it starts.
  wait 1 sec
  %echo% Scanner says, 'Beginning spectral analysis.'
  wait 1 sec
  %echo% Scanner says, 'Retrieving data...'
  wait 1 sec
  if %actor.affect(BLIND)% || %actor.affect(INVIS)% || %actor.affect(CURSE)% || %actor.affect(POISON)%
    %echo% Scanner says, 'Anomaly Detected!'
  else
    %echo% Scanner says, 'All clear'
  end
  wait 1 sec
  %send% %actor% The scanner stops and release you standing.
  %echoaround% %actor% The scanner stops and release %actor.name% standing.
  nop %actor.gold(-500)%
else
  return 0
end
~
#65464
witch love potion drink~
1 s 0
~
%send% %actor% A warm flush spreads through you; you feel utterly charming.
dg_affect %actor% CHA 5 120
~
#65465
witch rejuvenation potion drink~
1 s 0
~
%send% %actor% Years melt away as vitality surges back into your body!
%damage% %actor% -500
~
#65466
witch diet potion drink~
1 s 0
~
%send% %actor% You feel lighter as excess weight simply melts away.
dg_affect %actor% CHAR_WEIGHT -20 240
~
#65467
doyen growth pills eat~
1 s 0
~
%send% %actor% You feel a stretching sensation as your body grows taller.
dg_affect %actor% CHAR_HEIGHT 10 240
~
#65468
Virility pills~
1 s 0
~
%send% %actor% You eat a pill.
attach 65368 %actor.id%
return 0
%load% obj %self.vnum% %actor% inv
%purge% %self%
~
#65469
doyen hair restorer~
1 s 0
~
%send% %actor% Your scalp tingles as a thick head of hair grows back.
%echoaround% %actor% %actor.name%'s hair grows back thick and full.
~
#65471
pheromones spray~
1 c 3
spray~
%send% %actor% You spray the pheromones over yourself; an alluring scent surrounds you.
%echoaround% %actor% %actor.name% sprays a strange perfume and an alluring scent fills the air.
dg_affect %actor% CHA 3 60
~
#65472
witch wisdom potion drink~
1 s 0
~
%send% %actor% Your mind sharpens to brilliant, genius clarity!
dg_affect %actor% INT 5 240
~
#65478
elixir of life drink~
1 s 0
~
%send% %actor% The elixir floods you with pure life energy!
%echoaround% %actor% %actor.name% drinks a golden elixir and looks completely renewed.
%damage% %actor% -1000
~
#65483
sword of truth attach~
1 j 100
~
attach 65383 %actor.id%
~
#65484
paladin necklace attach~
1 ab 100
~
set actor %self.worn_by%
set card %actor.eq(hold)%
if %card% && %card.vnum(40097)%
  %load% obj 40096 %actor% inv
  %send% %actor% Your Paladin's Necklace transforms back the %card.shortdesc% into %actor.inventory.shortdesc%.
  %echoaround% %actor% %actor.name%'s paladin necklace transforms back %actor.hisher% %card.shortdesc% into %actor.inventory.shortdesc%.
  %purge% %card%
end
~
#65492
flute of confusion play~
1 c 3
play~
if %self.name% /= %arg%
%send% %actor% You play a strange, disorienting melody on the flute.
%echoaround% %actor% %actor.name% plays a bizarre melody that scrambles your senses!
set p %actor.room.people%
while %p%
set nextp %p.next_in_room%
if %p% != %actor%
dg_affect %p% CURSE on 15
end
set p %nextp%
done
else
return 0
end
~
#65493
haze smoke bomb throw~
1 c 2
throw~
if %self.name% /= %arg%
%send% %actor% You hurl the smoke bomb; thick haze blankets the area!
%echoaround% %actor% %actor.name% hurls a smoke bomb and thick, disorienting haze fills the room!
set p %actor.room.people%
while %p%
set nextp %p.next_in_room%
if %p% != %actor%
dg_affect %p% BLIND on 15
end
set p %nextp%
done
%purge% %self%
else
return 0
end
~
$~
