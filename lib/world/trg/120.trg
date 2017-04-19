#12000
Near Death Trap Lions - 12017~
2 g 100
~
* Near Death Trap stuns actor
wait 1 sec
set stunned %actor.hitp% 
%damage% %actor% %stunned%
wait 5 sec
%send% %actor% The lions grow bored once you stop struggling and leave you to die.
~
#12001
Magic User - 12009, 20, 25, 30-32~
0 k 10
~
switch %actor.level%
  case 1
  case 2
  case 3
  break
  case 4
    dg_cast 'magic missile' %actor%
  break
  case 5
    dg_cast 'chill touch' %actor%
  break
  case 6
    dg_cast 'burning hands' %actor%
  break
  case 7
  case 8
    dg_cast 'shocking grasp' %actor%
  break
  case 9
  case 10
  case 11
    dg_cast 'lightning bolt' %actor%
  break
  case 12
    dg_cast 'color spray' %actor%
  break
  case 13
    dg_cast 'energy drain' %actor%
  break
  case 14
    dg_cast 'curse' %actor%
  break
  case 15
    dg_cast 'poison' %actor%
  break
  case 16
    if %actor.align% > 0
      dg_cast 'dispel good' %actor%
    else
      dg_cast 'dispel evil' %actor%
    end
 break
  case 17
  case 18
    dg_cast 'call lightning' %actor%
  break
  case 19
  case 20
  case 21
  case 22
    dg_cast 'harm' %actor%
  break
  default
    dg_cast 'fireball' %actor%
  break
done
~
#12002
Cityguard - 12018, 21~
0 b 50
~
if !%self.fighting%
  set actor %random.char%
  if %actor%
    if %actor.is_killer%
      emote screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'
      kill %actor.name%
    elseif %actor.is_thief%
      emote screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'
      kill %actor.name%
    elseif %actor.cha% < 6
      %send% %actor% %self.name% spits in your face.
      %echoaround% %actor% %self.name% spits in %actor.name%'s face.
    end
    if %actor.fighting%
      eval victim %actor.fighting%
      if %actor.align% < %victim.align% && %victim.align% >= 0
        emote screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'
        kill %actor.name%
      end
    end
  end
end
~
#12003
Gladiator vs Lion~
0 g 100
~
wait 1 sec
switch %random.3%
  case 1
    say Kill the lion!
  break
  case 2
    say Get him!
  break
  case 3
    say This lion cannot stay alive!
  break
done
wait 3 sec
kill lion
~
#12004
junk received item~
0 j 100
~
%send% %actor% You give %object.shortdesc% to %self.name%.
%echoaround% %actor% %actor.name% gives %object.shortdesc% to %self.name%.
%purge% %object%
~
#12009
Heal players~
0 ab 20
~
set player %random.char%
if %player.hitp% < %player.maxhitp% && %player.is_pc% /= 1
dg_cast 'heal' %player.name%
dg_cast 'remove poison' %player.name%
dg_cast 'exorcise' %player.name%
say %player.name% is Healed!
else
dg_cast 'remove poison' %player.name%
dg_cast 'exorcise' %player.name%
end
~
#12012
peedler talk a little~
0 g 50
~
if %actor.is_pc%
wait 1 sec
say hey you, can you buy something? or help me some money?
end
~
#12013
peedler advice~
0 m 1
~
if %amount% >= 1
wait 1 sec
say Thank you very much!
wait 2 sec
say You can drink for free in this fountain...
wait 2 sec
say And take a seat in Meowth Restaurant to receive free food too!
wait 1 sec
laugh
wait 1 sec
say You can train your abilities with a nen master! go all north and east!
end
~
#12015
Room Zone Number~
2 bg 100
~
set room %room.vnum% 
eval number %room.strlen% 
switch %number% 
  case 3 
    set zone %room.charat(1)% 
  break 
  case 4 
    set 1st %room.charat(1)% 
    set 2nd %room.charat(2)% 
    set zone %1st%%2nd% 
  break 
  case 5 
    set 1st %room.charat(1)% 
    set 2nd %room.charat(2)% 
    set 3rd %room.charat(3)% 
    set zone %1st%%2nd%%3rd% 
  break 
done 
%echo% Room #%room.vnum% is part of zone: %zone% 
~
#12022
judge time~
0 g 100
~
if %direction% == west && %actor.is_pc%
close door
lock door
wait 1 sec
say Welcome to Rock-Paper-Scissors Tournament!
wait 4 sec
say Open every month, day 15 until we have a winner.
wait 4 sec
say Well we have 3 types of challanges games here.
wait 4 sec
say Jokenpo, combat or dice rolling!
wait 4 sec
say Which of these do you prefer?
attach 12023 %self.id%
end
~
#12023
games~
0 d 100
*~
if %actor.is_pc%
if %speech% == jokenpo
wait 1 sec
say Alright, Jokenpo selected!
wait 4 sec
say You have only to select between 	Grock	n, 	Gpaper	n or 	Gscissors	n.
wait 2 sec
emote stands in position to play.
wait 2 sec
say When you ready!
attach 12027 %self.id%
detach 12023 %self.id%
elseif %speech% == combat
wait 1 sec
say Sorry but this option is not available today.
elseif %speech% == dice || rolling
wait 1 sec
say Sorry but this option is not available today.
else
wait 1 sec
say Sorry but I did not understand, could you repeat that?
end
end
~
#12024
antokiba arena day 15~
0 ab 100
~
if %time.day% == 15
unlock door
open door
detach 12024 %self.id%
end
~
#12025
not day 15~
0 ab 100
~
if %time.day% != 15
attach 12024 %self.id%
detach 12025 %self.id%
end
~
#12026
mana potion~
1 s 100
~
if %actor.mana% < %actor.maxmana%
  set m 30
  nop %actor.mana(%m%)%
  if %actor.mana% > %actor.maxmana%
    eval m %actor.maxmana% - %actor.mana%
    nop %actor.mana(%m%)%
  end
end
if %actor.move% < %actor.maxmove%
  set v 30
  nop %actor.move(%v%)%
  if %actor.move% > %actor.maxmove%
    eval v %actor.maxmove% - %actor.move%
    nop %actor.move(%v%)%
  end
end
%send% %actor% You feel energized.
%send% %actor% You feel refreshed.
~
#12027
rock paper scissors~
0 c 100
*~
if !%actor.is_pc%
return 0
halt
end
switch %cmd%
case rock
%force% %actor% say JO...
say JO...
wait 1 sec
%force% %actor% say KEN...
say KEN...
wait 1 sec
%force% %actor% say PO!
say PO!
%echoaround% %actor% %actor.name% played rock.
eval npc %random.3%
if %actor.varexists(good_luck)%
set npc 3
rdelete good_luck %actor.id%
end
if %npc% == 1
%echo% %self.name% played rock too.
wait 1 sec
say Oh noes... it was a draw!
wait 4 sec
say Show me your next hand when you ready!
halt
elseif %npc% == 2
%echo% %self.name% played paper.
wait 1 sec
say Sorry but you get nothing... you lose! Good month sir.
wait 4 sec
%teleport% %actor% 12009
%force% %actor% look
attach 12025 %self.id%
detach 12027 %self.id%
halt
else
%echo% %self.name% played scissors.
wait 1 sec
say We have a winner! Congratulations!
wait 1 sec
say Take your reward.
eval prize 65302 + %random.15%
set rcard %prize%
%load% obj %rcard% %self%
give card %actor.name%
wait 4 sec
%teleport% %actor% 12009
%force% %actor% look
attach 12025 %self.id%
detach 12027 %self.id%
halt
end
brake
case paper
%force% %actor% say JO...
say JO...
wait 1 sec
%force% %actor% say KEN...
say KEN...
wait 1 sec
%force% %actor% say PO!
say PO!
%echoaround% %actor% %actor.name% played paper.
eval npc %random.3%
if %actor.varexists(good_luck)%
set npc 3
rdelete good_luck %actor.id%
end
if %npc% == 1
%echo% %self.name% played paper too.
wait 1 sec
say Oh noes... it was a draw!
wait 4 sec
say Show me your next hand when you ready!
halt
elseif %npc% == 2
%echo% %self.name% played scissors.
wait 1 sec
say Sorry but you get nothing... you lose! Good month sir.
wait 4 sec
%teleport% %actor% 12009
%force% %actor% look
attach 12025 %self.id%
detach 12027 %self.id%
halt
else
%echo% %self.name% played rock.
wait 1 sec
say We have a winner! Congratulations!
wait 1 sec
say Take your reward.
eval prize 65302 + %random.15%
set rcard %prize%
%load% obj %rcard% %self%
give card %actor.name%
wait 4 sec
%teleport% %actor% 12009
%force% %actor% look
attach 12025 %self.id%
detach 12027 %self.id%
halt
end
brake
case scissors
%force% %actor% say JO...
say JO...
wait 1 sec
%force% %actor% say KEN...
say KEN...
wait 1 sec
%force% %actor% say PO!
say PO!
%echoaround% %actor% %actor.name% played scissors.
eval npc %random.3%
if %actor.varexists(good_luck)%
set npc 3
rdelete good_luck %actor.id%
end
if %npc% == 1
%echo% %self.name% played scissors too.
wait 1 sec
say Oh noes... it was a draw!
wait 4 sec
say Show me your next hand when you ready!
halt
elseif %npc% == 2
%echo% %self.name% played rock.
wait 1 sec
say Sorry but you get nothing... you lose! Good month sir.
wait 4 sec
%teleport% %actor% 12009
%force% %actor% look
attach 12025 %self.id%
detach 12027 %self.id%
halt
else
%echo% %self.name% played paper.
wait 1 sec
say We have a winner! Congratulations!
wait 1 sec
say Take your reward.
eval prize 65302 + %random.15%
set rcard %prize%
%load% obj %rcard% %self%
give card %actor.name%
wait 4 sec
%teleport% %actor% 12009
%force% %actor% look
attach 12025 %self.id%
detach 12027 %self.id%
halt
end
brake
default
return 0
brake
done
~
#12035
smoke bomb~
1 h 100
~
dg_affect %actor% infra on 1
dg_cast 'darkness'
set i %actor.room.people%
while %i%
set next %i.next_in_room%
if %i% != %actor%
dg_affect %i% blind 0 1
end
set i %next%
done
%purge% %self%
~
#12036
cat diner event~
0 e 0
sits down upon a chair.~
wait 2 sec
say Meowelcome to my restaurant.
wait 4 sec
say Today who eat two platefuls of pasta at once will not pay!
wait 4 sec
say and plus will receive a 	yF-185 Galgaida card	n!
wait 4 sec
say did you nian accept the challange?
attach 12037 %self.id%
~
#12037
player say yes~
0 d 100
yes~
if %actor.pos% == SITTING && %actor.is_pc%
wait 1 sec
say MeOK! Here is it, but I don't know if you nian can do it.
%load% obj 3000 %self%
%load% obj 3000 %self%
give plateful %actor.name%
give plateful %actor.name%
wait 2 sec
say You do not have so meotch time to eat it, so go fast!
wait 2 sec
say or will gonna pay for that...
wait 2 sec
grin %actor.name%
wait 2 sec
say TIME IS MEOWVER!
look %actor.name%
if %actor.hunger% >= 23 && !%actor.has_item(3000)%
wait 2 sec
say I CAN NOT BELIEVE! YOU DID IT!
wait 2 sec
shake %actor.name%
wait 2 sec
say Congratulations! Here, takes this card as prize.
%load% obj 40098 %self%
give card %actor.name%
detach 12037 %self.id%
else
say Oh meow... you loose...
emote deducts 200 coins from %actor.name% as payment.
nop %actor.gold(-100)%
detach 12037 %self.id%
end
else
detach 12037 %self.id%
end
~
#12040
player ask~
0 d 1
master practice hint advice direction hi hello where skill hatsu~
wait 1 sec
say the nen master is in the backyard, south from here kid.
~
#12041
players wrong ask~
0 d 1
*~
if %self.bartender_asked% != %actor.id% && %actor.is_pc%
set bartender_asked %actor.id%
global bartender_asked
wait 1 sec
say what did you want, kid?
end
~
#12042
bartender advice~
0 m 1
~
if %amount% >= 1
if %actor.sex% == male
wait 1 sec
say Thanks for the tip sir.
elseif %actor.sex% == female
wait 1 sec
say Thanks for the tip madam.
end
wait 2 sec
say If you need a "escape route", use the secret passageway to northeast.
end
~
#12043
10000 coins gain~
1 c 3
ga~
if %self.carried_by% == %actor%
  %send% %actor% You must hold something before gain it.
  halt
end
eval hold %actor.eq(hold)%
if %cmd.mudcommand%==gain && %hold.id% /= %self.id%
  %force% %actor% say GAIN!
  wait 1 sec
  %send% %actor% Your %self.shortdesc% turns into 10000 jenny coins.
  %echoaround% %actor% %actor.name%'s %self.shortdesc% turns into a lot of gold coins.
  nop %actor.gold(10000)%
  %purge% %self%
else
  return 0
end
~
#12053
limit load map~
0 ab 100
~
%at% 12053 get all
~
#12064
visited antokiba tag~
2 q 100
~
if %actor.is_pc%
rdelete visited_antokiba %actor.id%
eval visited_antokiba %self.vnum%
remote visited_antokiba %actor.id%
end
~
#12099
test~
0 ab 100
~
if %self.room.vnum% == 2808
detach 12099 %self.id%
else
%transform% 12039
detach 12099 %self.id%
end
~
$~
