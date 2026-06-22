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
angel~
0 n 100
~
wait 5 sec
say 	CTell me, who you want me to heal?	n
attach 3249 %self.id%
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
#65434
fledgling egg reset on remove~
1 l 100
~
osetval 0 0
%send% %actor% The egg cools in your grip; its incubation must begin again.
~
#65446
gold dust girl action~
0 n 100
~
wait 1 sec
emote sheds a cascade of golden dust, then dissolves into sparkling motes.
%force% %self% drop coins
%purge% %self%
~
#65447
sleeping girl lifespan~
0 n 100
~
wait 1800 sec
if !%self.room%
  halt
end
%echo% The sleeping girl stirs softly, mumbles, and fades like a vanishing dream.
%purge% %self%
~
#65448
aromatherapy girl heal~
0 b 100
~
:loop
wait 75 sec
if !%self.master%
  halt
end
eval master %self.master%
if %master.pos% == Incapacitated || %master.pos% == MortallyWounded || %master.pos% == Dead || %master.pos% == Stunned
  goto loop
end
eval heal_hp %master.maxhp% / 10
eval heal_mana %master.maxmana% / 10
%damage% %master% -%heal_hp%
nop %master.mana(%master.mana% + %heal_mana%)%
%send% %master% The gentle aroma from %self.name% soothes your stress a little.
goto loop
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
#65470
mad scientist steroids drink~
1 s 0
~
%send% %actor% You force down the foul steroid brew; your muscles swell.
dg_affect %actor% STR 2 240
~
#65471
pheromones spray~
1 c 3
spray~
%send% %actor% You spray the pheromones over yourself; an alluring scent surrounds you.
%echoaround% %actor% %actor.name% sprays a strange perfume and an alluring scent fills the air.
dg_affect %actor% CHA 3 60
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
#65486
quiver of frustration loose~
1 c 2
loose~
if %self.val0% <= 0
%send% %actor% The quiver of frustration is empty; no arrows remain.
halt
end
eval arrows %self.val0% - 1
osetval 0 %arrows%
%send% %actor% You loose an arrow; a Leave card forms in your grip. (%arrows% arrows left)
%echoaround% %actor% %actor.name% looses an arrow from a quiver of frustration.
%load% obj 1014 %actor% inv
~
#65496
clairvoyant snake summon~
1 abn 100
~
if %self.carried_by% || %self.worn_by%
%load% mob 65496
%echo% A clairvoyant snake uncoils from the card and slithers to your side.
%purge% %self%
end
~
#65497
clairvoyant snake follow~
0 n 100
~
set p %self.room.people%
while %p%
if %p.is_pc%
mfollow %p%
halt
end
set p %p.next_in_room%
done
~
#65498
clairvoyant snake feed~
0 c 100
feed~
if !(%self.name% /= %arg%)
return 0
halt
end
if %self.lastfed% == %time.day%
%send% %actor% %self.name% is sated; it can only be fed once a day.
halt
end
set lastfed %time.day%
remote lastfed %self.id%
if %actor.cardcount(1015)% >= 70
%send% %actor% %self.name% writhes, but no card appears -- the world already holds every Clairvoyance.
halt
end
%send% %actor% You feed %self.name%, which shudders and spits up a Clairvoyance card!
%echoaround% %actor% %actor.name% feeds a clairvoyant snake, which spits up a card.
%load% obj 1015 %actor% inv
~
#65499
shield faith wear~
1 j 100
~
eval shield_faith 1
remote shield_faith %actor.id%
%send% %actor% The Shield of Faith pulses with protective light.
~
#65500
shield faith remove~
1 l 100
~
rdelete shield_faith %actor.id%
%send% %actor% The Shield of Faith dims as you lower it.
~
#65501
tax gauntlet levy~
1 c 1
levy~
* Only fire when wearer actually typed 'levy'
if %cmd.mudcommand% != levy
  return 0
  halt
end
* Gauntlet must be worn/held by actor
if !%self.worn_by% || %self.worn_by% != %actor%
  if !%self.carried_by% || %self.carried_by% != %actor%
    return 0
    halt
  end
end
* Binder check
eval binder %actor.inventory(3203)%
if !%binder%
  %send% %actor% You need a binder to use the gauntlet.
  halt
end
* Count restricted cards in actor's binder
eval n 0
eval o %binder.contents%
while %o%
  eval nexto %o.next_in_list%
  if %o.type% == RESTRICTED
    eval n %n% + 1
  end
  eval o %nexto%
done
if %n% == 0
  %send% %actor% The gauntlet is ineffective -- no restricted card remains as tribute.
  halt
end
* Pick random restricted card to consume as tribute
eval pick %random.%n%%
eval c 0
eval tribute 0
eval o %binder.contents%
while %o%
  eval nexto %o.next_in_list%
  if %o.type% == RESTRICTED
    eval c %c% + 1
    if %c% == %pick%
      eval tribute %o%
    end
  end
  eval o %nexto%
done
if !%tribute%
  %send% %actor% The gauntlet flickers but nothing happens.
  halt
end
%send% %actor% You sacrifice %tribute.shortdesc% to the gauntlet!
%echoaround% %actor% %actor.name%'s gauntlet flares and consumes a card!
%purge% %tribute%
%force% %actor% say Levy ON!
* Now apply forced Levy to all non-allied PCs in room (bypass defenses)
eval i %actor.room.people%
while %i%
  eval nexti %i.next_in_room%
  if %i.is_pc% == 1 && %i% != %actor%
    eval other_id %i.id%
    eval allied %actor.same_group(%other_id%)%
    if !%allied%
      eval bv %i.inventory(3203)%
      if %bv%
        eval max 0
        eval u %bv.contents%
        while %u%
          eval nextu %u.next_in_list%
          eval max %max% + 1
          eval u %nextu%
        done
        if %max% == 0
          %send% %actor% %i.name% has no cards to steal.
        elseif %actor.varexists(good_luck)%
          * good_luck: steal first restricted card
          eval u %bv.contents%
          eval stolen 0
          while %u% && !%stolen%
            eval nextu %u.next_in_list%
            if %u.type% == RESTRICTED
              %load% obj %u.vnum% %actor% inv
              %send% %actor% You forcibly levied %u.shortdesc% from %i.name%!
              %send% %i% %actor.name% forcibly levied a card from you!
              %echoaround% %actor% %actor.name% casts a forced Levy on %i.name%.
              %purge% %u%
              eval stolen 1
            end
            if !%stolen%
              eval u %nextu%
            end
          done
        else
          * normal: steal random card
          eval steal %random.%max%%
          eval counter 0
          eval y %bv.contents%
          while %y%
            eval nexty %y.next_in_list%
            eval counter %counter% + 1
            if %steal% == %counter%
              %load% obj %y.vnum% %actor% inv
              %send% %actor% You forcibly levied %y.shortdesc% from %i.name%!
              %send% %i% %actor.name% forcibly levied a card from you!
              %echoaround% %actor% %actor.name% casts a forced Levy on %i.name%.
              %purge% %y%
              eval y 0
            else
              eval y %nexty%
            end
          done
        end
      else
        %send% %actor% %i.name% has no binder.
      end
    end
  end
  eval i %nexti%
done
rdelete good_luck %actor.id%
~
#65502
secret cape wear~
1 j 100
~
eval blackout_curtain_cape 1
remote blackout_curtain_cape %actor.id%
%send% %actor% The Secret Cape drapes over you; you feel hidden from prying spells.
~
#65503
secret cape remove~
1 l 100
~
rdelete blackout_curtain_cape %actor.id%
%send% %actor% The Secret Cape falls away; you feel exposed.
~
#65510
hot spring heal loop~
1 n 100
~
* Fires once on load; heals all PCs in room every 60 sec for 30 min (1800 sec).
if %self.spring_active%
  halt
end
remote spring_active %self.id%
set elapsed 0
:loop
wait 60 sec
if !%self.room%
  halt
end
set p %self.room.people%
while %p%
  if %p.is_pc% && %p.hp% < %p.maxhp%
    %damage% %p% -10
  end
  set p %p.next_in_room%
done
set elapsed %elapsed% + 60
if %elapsed% >= 1800
  %echo% The hot spring cools and gradually sinks back into the earth.
  %purge% %self%
  halt
end
goto loop
~
#65511
liquor spring destruct~
1 n 100
~
* Fires once on load; removes the spring after 30 min (1800 sec).
if %self.spring_active%
  halt
end
remote spring_active %self.id%
wait 1800 sec
if !%self.room%
  halt
end
%echo% The liquor spring runs dry and disappears into the earth.
%purge% %self%
~
#65512
mystery pond fish spawn~
1 n 100
~
* Fires once on load; spawns fish immediately, then replenishes every 1800 sec.
if %self.pond_active%
  halt
end
remote pond_active %self.id%
gosub spawn
:loop
wait 1800 sec
if !%self.room%
  halt
end
gosub spawn
goto loop
:spawn
* Count each fish type and spawn up to 2 of each
set c1217 0
set c3702 0
set c10006 0
set c10102 0
set c27200 0
set c27516 0
set c31511 0
set c31581 0
set c31582 0
set item %self.contents%
while %item%
  set nx %item.next_in_list%
  if %item.vnum% == 1217
    set c1217 %c1217% + 1
  elseif %item.vnum% == 3702
    set c3702 %c3702% + 1
  elseif %item.vnum% == 10006
    set c10006 %c10006% + 1
  elseif %item.vnum% == 10102
    set c10102 %c10102% + 1
  elseif %item.vnum% == 27200
    set c27200 %c27200% + 1
  elseif %item.vnum% == 27516
    set c27516 %c27516% + 1
  elseif %item.vnum% == 31511
    set c31511 %c31511% + 1
  elseif %item.vnum% == 31581
    set c31581 %c31581% + 1
  elseif %item.vnum% == 31582
    set c31582 %c31582% + 1
  end
  set item %nx%
done
if %c1217% < 2
  %load% obj 1217 %self% obj
end
if %c3702% < 2
  %load% obj 3702 %self% obj
end
if %c10006% < 2
  %load% obj 10006 %self% obj
end
if %c10102% < 2
  %load% obj 10102 %self% obj
end
if %c27200% < 2
  %load% obj 27200 %self% obj
end
if %c27516% < 2
  %load% obj 27516 %self% obj
end
if %c31511% < 2
  %load% obj 31511 %self% obj
end
if %c31581% < 2
  %load% obj 31581 %self% obj
end
if %c31582% < 2
  %load% obj 31582 %self% obj
end
return
~
#65513
tree of plenty fruit spawn~
1 n 100
~
* Fires once on load; spawns one of each fruit, then replenishes missing ones every 1800 sec.
if %self.tree_active%
  halt
end
remote tree_active %self.id%
gosub spawn
:loop
wait 1800 sec
if !%self.room%
  halt
end
gosub spawn
goto loop
:spawn
* Spawn one of each fruit type if not already present
set c111 0
set c1400 0
set c635 0
set c31727 0
set c1927 0
set c7508 0
set c11828 0
set c4524 0
set c25600 0
set c25610 0
set item %self.contents%
while %item%
  set nx %item.next_in_list%
  if %item.vnum% == 111
    set c111 %c111% + 1
  elseif %item.vnum% == 1400
    set c1400 %c1400% + 1
  elseif %item.vnum% == 635
    set c635 %c635% + 1
  elseif %item.vnum% == 31727
    set c31727 %c31727% + 1
  elseif %item.vnum% == 1927
    set c1927 %c1927% + 1
  elseif %item.vnum% == 7508
    set c7508 %c7508% + 1
  elseif %item.vnum% == 11828
    set c11828 %c11828% + 1
  elseif %item.vnum% == 4524
    set c4524 %c4524% + 1
  elseif %item.vnum% == 25600
    set c25600 %c25600% + 1
  elseif %item.vnum% == 25610
    set c25610 %c25610% + 1
  end
  set item %nx%
done
if !%c111%
  %load% obj 111 %self% obj
end
if !%c1400%
  %load% obj 1400 %self% obj
end
if !%c635%
  %load% obj 635 %self% obj
end
if !%c31727%
  %load% obj 31727 %self% obj
end
if !%c1927%
  %load% obj 1927 %self% obj
end
if !%c7508%
  %load% obj 7508 %self% obj
end
if !%c11828%
  %load% obj 11828 %self% obj
end
if !%c4524%
  %load% obj 4524 %self% obj
end
if !%c25600%
  %load% obj 25600 %self% obj
end
if !%c25610%
  %load% obj 25610 %self% obj
end
return
~
#65514
hormone cookie consume~
1 s 0
~
* NPCs are immune even if forced to eat
if %actor.is_npc%
  halt
end
* Re-eat during active effect: cookie is consumed but no effect applies
if %actor.hormone_active%
  %send% %actor% The cookie dissolves on your tongue, but your body is already shifting -- there is nothing more to do.
  halt
end
* Only Male/Female are affected; Neutral characters eat it with no result
if %actor.sex% == Male
  eval orig_sex 1
  nop %actor.sex(2)%
  %send% %actor% You bite into the cookie. A strange warmth spreads through you as your body softly reshapes itself...
  %echoaround% %actor% %actor.name% eats a small cookie -- and their features begin to shift in unexpected ways.
elseif %actor.sex% == Female
  eval orig_sex 2
  nop %actor.sex(1)%
  %send% %actor% You bite into the cookie. A strange warmth spreads through you as your body firmly reshapes itself...
  %echoaround% %actor% %actor.name% eats a small cookie -- and their features begin to shift in unexpected ways.
else
  %send% %actor% You bite into the cookie -- but it seems to have no effect. Perhaps your body needs a defined sex for the magic to take hold.
  halt
end
* Mark effect as active and store original sex on the player
eval hormone_active 1
remote hormone_active %actor.id%
eval hormone_orig_sex %orig_sex%
remote hormone_orig_sex %actor.id%
* Attach reversion timer (24 MUD hours = 1800 real seconds)
attach 65515 %actor.id%
~

#65515
hormone reversion timer~
0 b 100
~
* Wait 24 MUD hours (1800 real seconds)
wait 1800 sec
* Safety: if variable is gone (e.g. partial reboot), clean up and exit
if !%self.hormone_active%
  detach 65515 %self.id%
  halt
end
* Revert to stored original sex
if %self.hormone_orig_sex% == 1
  nop %self.sex(1)%
  %send% %self% The cookie effects fade. Your body quietly returns to its natural male form.
  %echoaround% %self% %self.name%'s features gradually settle back to their original form.
else
  nop %self.sex(2)%
  %send% %self% The cookie effects fade. Your body quietly returns to its natural female form.
  %echoaround% %self% %self.name%'s features gradually settle back to their original form.
end
rdelete hormone_active %self.id%
rdelete hormone_orig_sex %self.id%
detach 65515 %self.id%
~

$~
