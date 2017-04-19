#3200
No drop~
1 h 100
~
if %actor.level% >= 31
return 1
else
%send% %actor% You can't keep playing without your %self.shortdesc%.
return 0
end
~
#3201
Purge object~
1 ab 1
~
if %self.is_inroom%
%echo% %self.shortdesc% vanishes on the ground.
%purge% %self%
else
return 0
end
~
#3202
G.I. Ring~
1 c 1
book~
if %cmd%==book && %actor.varexists(book_purge_leaves)%
  if %actor.room.contents.vnum% == 3200
    return 0
    halt
  else
    %force% %actor% say BOOK!
    wait 1 sec
    %force% %actor% open binder
    %send% %actor% An energy comes out of your ring and transforms into a book.
    %echoaround% %actor% An energy comes out of %actor.name%'s ring and transforms into a book.
    halt
  end
elseif %cmd%==book
  %force% %actor% say BOOK!
  wait 1 sec
  eval book_purge_leaves %actor.room.contents%
  remote book_purge_leaves %actor.id%
  %force% %actor% open binder
  %send% %actor% An energy comes out of your ring and transforms into a book.
  %echoaround% %actor% An energy comes out of %actor.name%'s ring and transforms into a book.
else
  return 0
end
~
#3203
No get~
1 g 100
~
if %actor.level% >= 31
return 1
else
%send% %actor% It's not belongs to you.
return 0
end
~
#3204
instapurge drop~
1 h 100
~
%echo% %self.shortdesc% vanishes on the ground.
%purge% %self%
~
#3205
book control~
1 q 100
~
if %actor.varexists(book_purge_leaves)%
  %echo% The book what was floating here closes itself and vanishes.
  set remove_book %actor.book_purge_leaves%
  rdelete book_purge_leaves %actor.id%
  %purge% %self%
end
~
#3206
gain card to obj 100~
1 c 1
ga~
eval hold %actor.eq(hold)%
if %cmd%==gain && %hold.id% /= %self.id%
eval card %self.vnum% + 100
%force% %actor% say GAIN!
wait 1 sec
%load% obj %card% %actor% inv
%send% %actor% Your %self.shortdesc% turns into %actor.inventory.shortdesc%.
%echoaround% %actor% %actor.name%'s %self.shortdesc% turns into %actor.inventory.shortdesc%.
%purge% %self%
else
return 0
end
~
#3207
gain obj to card 100~
1 c 1
ga~
eval hold %actor.eq(hold)%
if %cmd%==gain && %hold.id% /= %self.id%
eval card %self.vnum% - 100
%force% %actor% say GAIN!
wait 1 sec
%load% obj %card% %actor% inv
%send% %actor% Your %self.shortdesc% turns into %actor.inventory.shortdesc%.
%echoaround% %actor% %actor.name%'s %self.shortdesc% turns into %actor.inventory.shortdesc%.
%purge% %self%
else
return 0
end
~
#3208
room container purge 1~
2 ab 100
~
%purge% corpse
%echo% The corpse of a dead player vanishes away...
detach 3209 %self.id%
detach 3208 %self.id%
~
#3209
room container purge 2~
2 c 100
g~
if %cmd.mudcommand% == get
%send% %actor% The corpse of dead player vanishes when you approaches.
%purge% corpse
detach 3208 %self.id%
detach 3209 %self.id%
else
return 0
end
~
#3210
player purge corpse~
0 f 100
~
attach 3209 %self.room.id%
attach 3208 %self.room.id%
~
#3211
detach greet~
2 g 100
~
wait 1 sec
detach 3210 %actor.id%
rdelete book_purge_leaves %actor.id%
~
#3212
GI ring no remove~
1 l 100
~
if %actor.level% >= 31
return 1
else
%send% %actor% You must keep %self.shortdesc% in your finger to keep playing properly.
return 0
end
~
#3213
book return~
1 c 4
book~
if %cmd%==book && %actor.varexists(book_purge_leaves)%

eval ringdesc %actor.eq(rfinger)%
%send% %actor% You returned your %self.shortdesc% into %ringdesc.shortdesc%.
%echoaround% %actor% %actor.name% return %actor.hisher% %self.shortdesc% into %ringdesc.shortdesc%.
rdelete book_purge_leaves %actor.id%
%purge% %self.id%
end
~
#3214
book expire~
1 f 100
~
%echo% The book what was floating here closes itself and vanishes.
rdelete book_purge_leaves %actor.id%
%purge% %self%
~
#3215
card purge time +100~
1 fhj 100
~
set oldsdesc %self.shortdesc%
eval revert %self.vnum% + 100
if %self.worn_by%
%transform% %revert%
%send% %self.owner_id% The %oldsdesc% that you were holding transforms into a %self.shortdesc%... FOREVER!
%echoaround% %self.owner_id% The %oldsdesc% that %self.owner_name% were holding transforms into a %self.shortdesc%... FOREVER!
eval do_revert 1
remote do_revert %self.id%
detach 3206 %self.id%
attach %self.vnum% %self.id%
detach 3215 %self.id%
halt
end
if %self.is_inroom%
%transform% %revert%
%echo% The %oldsdesc% was lying on the ground turns into a %self.shortdesc%... FOREVER.
eval do_revert 1
remote do_revert %self.id%
detach 3206 %self.id%
attach %self.vnum% %self.id%
detach 3215 %self.id%
end
eval owner_id %actor%
eval owner_name %actor.name%
remote owner_id %self.id%
remote owner_name %self.id%
otimer 2
~
#3216
spell purge timer~
1 fhj 100
~
if %self.worn_by%
%echo% A held %self.shortdesc% magic fizzles out and dies.
%purge% %self%
elseif %self.is_inroom%
%echo% A %self.shortdesc% what was laying here suddenly disappears.
%purge% %self%
end
otimer 2
~
#3217
free binder rules~
1 c 2
p~
if %cmd.mudcommand% == put && %self.name% /= %arg.cdr%
if !%has_it%
eval i %actor.inventory%
while %i%
set next %i.next_in_list%
if %i.type%==UNRESTRICTED && %i.name% /= %arg.car%
set has_it 1
break
end
set i %next%
done
end
if %has_it%
%force% %actor% put %arg.car% in %self.name%
else
%echo% Only un-restricted cards fits there.
end
else
return 0
end
~
#3218
spell binder rules~
1 c 2
p~
if %cmd.mudcommand% == put && %self.name% /= %arg.cdr%
if !%has_it%
eval i %actor.inventory%
while %i%
set next %i.next_in_list%
if %i.type%==SPELLCARD && %i.name% /= %arg.car%
set has_it 1
break
end
set i %next%
done
end
if %has_it%
%force% %actor% put %arg.car% in %self.name%
else
%echo% Only spell cards fits there.
end
else
return 0
end
~
#3219
spell specified rules~
1 c 2
p~
if %cmd.mudcommand% == put && %self.name% /= %arg.cdr%
if !%has_it%
eval i %actor.inventory%
while %i%
set next %i.next_in_list%
if %i.type%==RESTRICTED && %i.name% /= %arg.car%
set has_it 1
break
end
set i %next%
done
end
if %has_it%
%force% %actor% put %arg.car% in %self.name%
else
%echo% Only specified cards fits there.
end
else
return 0
end
~
#3220
No drop~
1 hi 100
~
if %actor.level% >= 31
return 1
else
%send% %actor% It is attached with your book.
return 0
end
~
#3221
no steal binders~
1 c 1
ste~
if %cmd.mudcommand% == steal
eval car %arg.car%
%echo% %car.vnum(3200).shortdesc%
else
return 0
end
~
#3222
binder rules~
1 c 2
put~
%echo% debug car: %arg.car%
%echo% debug cdr: %arg.cdr%
if %cmd.mudcommand% == put
set argmaster in %self.name%
%echo% argmaster: %argmaster%
set card %arg.car%
if %arg.cdr% == %argmaster% || %self.name%
else
%echo% reach else1 %self.name%
%force% %actor% put %arg.car% %arg.cdr%
halt
end
eval i %actor.inventory(3203).contents%
while %i%
set next %i.next_in_list%
if %i.vnum% == %card.vnum%
break
end
set i %next%
done
%echo% debug card %card.vnum%
%echo% debug i %i.shortdesc%
if !%i%
%echo% reach if i %self.name%
%force% %actor% put %arg.car% %self.name%
else
%send% %actor% The card slot is already occupied.
end
else
%echo% reach else2 %self.name%
%force% %actor% put %arg.car% %arg.cdr%
else
return 0
end
~
#3223
mob hints~
0 ab 10
~
switch %random.23%
case 1
say Masadora is in the southern part of the island, between Antokiba and Aiai.
break
case 2
say There is two ways to leave this island, one of them is using spell card Leave.
break
case 3
say Other way to leave this island is bribing the dock chief near Masadora harbor...
wait 1 sec
say But he charges very different amounts every day for a passage. Or "just" beat him.
break
case 4
say The best place to buy spell cards is in Masadora and you can sell in almost any city.
break
case 5
say Sometimes monsters are very difficult and dangerous, try 	Gconsider	n it.
break
case 6
say Is better 	Gflee	n from a combat than die fighting.
break
case 7
say Keep your cards into your binder, or someone skilled can steal it from you.
break
case 8
say Do not leave a card on the ground or hold it for more than 1 minute or you lose it.
break
case 9
say The only way to steal a card inside a player binder is using a spell card.
break
case 10
say You are not a 	Gwimp	n when you are being cautious.
break
case 11
say Every day 15 in Antokiba you can participate in a rock-paper-scissors contest.
break
case 12
say Some events can give you valuable cards or even restricted ones.
break
case 13
say Only players uses 	GG.I. 	DRing	n, in doubt look first.
break
case 14
say If you even try to kill another character, you can be appointed as a killer.
break
case 15
say Some events are hidden, waiting your actions to trigger it.
break
case 16
say When a player dies, all belongings disappears too.
break
case 17
say This game can be less difficult if you play as a 	Ggroup	n.
break
case 18
say If you want to accompany a friend, just 	Gfollow	n him.
break
case 19
say You can 	Gsplit	n rewards with your group, after all they deserve too.
break
case 20
say You can 	Gpractice	n your skills with a Nen Master!
break
case 21
say The lower your armor class is, lower chances the enemy has to hit you.
break
case 22
say You can keep your money safe, just 	Gdeposit	n it with a teller machine.
break
case 23
say Some places has secret passageways, you can find it if is a good observer.
break
default
break
done
~
#3224
book purge no pc in room~
1 ab 100
~
* No Script
~
#3225
book no open~
1 c 2
o~
if %cmd.mudcommand% == open && %self.name% /= %arg%
if %actor.varexists(book_purge_leaves)%
%force% %actor% open binder
else
%send% %actor% You need call your book to access the binder slots.
end
else
return 0
end
~
#3226
spell card activated~
1 hij 100
~
%send% %actor% %self.shortdesc% is binded to your book while activated.
return 0
~
#3227
hold ignore~
1 c 2
ho~
if %cmd.mudcommand% == hold && %self.name% /= %arg%
return 0
else
return 0
end
~
#3228
reply command~
1 c 1
r~
if %cmd.mudcommand% == reply
if %actor.varexists(contact)%
%force% %actor% tell %arg%
halt
else
%send% %actor% You have nobody to reply to!
halt
end
else
return 0
end
~
#3229
transform card~
1 ab 100
~
detach %self.vnum% %self.id%
set oldsdesc %self.shortdesc%
eval revert %self.vnum%
remote revert %self.id%
%transform% %self.transform_card%
%send% %self.carried_by% The %oldsdesc% transformed into %self.shortdesc%.
detach 3229 %self.id%
~
#3230
card purge time -100~
1 fhj 100
~
set oldsdesc %self.shortdesc%
eval revert %self.vnum% - 100
if %self.worn_by%
%transform% %revert%
%send% %self.owner_id% The %oldsdesc% that you were holding transforms into a %self.shortdesc%... FOREVER!
%echoaround% %self.owner_id% The %oldsdesc% that %self.owner_name% were holding transforms into a %self.shortdesc%... FOREVER!
eval do_revert 1
remote do_revert %self.id%
detach all %self.id%
halt
end
if %self.is_inroom%
%transform% %revert%
%echo% The %oldsdesc% was lying on the ground turns into a %self.shortdesc%... FOREVER.
eval do_revert 1
remote do_revert %self.id%
detach all %self.id%
end
eval owner_id %actor%
eval owner_name %actor.name%
remote owner_id %self.id%
remote owner_name %self.id%
otimer 2
~
#3231
no recite~
1 c 1
rec~
if %cmd.mudcommand% == recite
%send% %actor% Nani!?!
else
return 0
end
~
#3232
no quit~
1 c 1
quit~
if %cmd.mudcommand% == quit
detach 2805 %self.id%
if %actor.room.vnum% == 1407
%force% %actor% rent
else
	C%send% %actor%	You only can quit in front of Elena, find the way out of the island first.	n
end
else
return 0
end
~
#3233
auto give elena~
1 ab 100
~
%force% %self.carried_by.id% give token elena
%purge% %self%
~
#3234
paladin necklace detach~
1 ab 100
~
if %self.revert% > 0
set oldsdesc %self.shortdesc%
set actor %self.worn_by%
set actorinv %self.carried_by%
%load% obj %self.revert% %actor% inv
%load% obj %self.revert% %actorinv% inv
%send% %actor% Your Paladin's Necklace transforms back the %oldsdesc% into %self.shortdesc%.;%echoaround% %actor% %actor.name%'s paladin necklace transforms back %actor.hisher% %oldsdesc% into %self.shortdesc%.
%send% %actorinv% Your Paladin's Necklace transforms back the %oldsdesc% into %self.shortdesc%.;%echoaround% %actorinv% %actorinv.name%'s paladin necklace transforms back %actorinv.hisher% %oldsdesc% into %self.shortdesc%.
detach 3234 %self.id%
end
detach 3234 %self.id%
~
#3235
wand staff no gain~
1 ab 100
~
if %self.val2% < %self.val1%
detach all %self.id%
end
~
#3236
trace command~
1 c 1
trace~
set target %arg.room.vnum%
set prevloc %actor.room.vnum%
if %cmd% == trace
if %arg.name% != %actor.name%
if %actor.varexists(trace_%arg.name%)% && %arg.varexists(trace_%arg.name%)%
if %target% == %actor.room.vnum%
%send% %actor% %arg.heshe% is here!
%force% %actor% look
else
%send% %actor% Tracing target...
%teleport% %actor% %target%
%force% %actor% look
%teleport% %actor% %prevloc%
%echoaround% %actor% %actor.name% feels informed.
halt
end
else
%send% %actor% No one was traced by that name.
end
else
%send% %actor% Trace yourself? WHY?!
end
else
return 0
end
~
#3237
tell command~
1 c 1
t~
if %cmd.mudcommand% == tell
if %actor.varexists(contact)%
%force% %actor% tell %actor.contact% %arg%
halt
else
%send% %actor% Ok but... how?
halt
end
else
return 0
end
~
#3238
adhesion command~
1 c 1
adhesion~
if %cmd% == adhesion
if %actor.varexists(adhesion_%arg.name%)% && %arg.varexists(adhesion_%arg.name%)%
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
else
%send% %actor% No one with adhesion by that name.
end
else
return 0
end
~
#3239
player met~
0 g 100
~
if %actor.is_pc% == 1
eval met_%actor.name% 1
eval met_%self.name% 1
remote met_%actor.name% %self.id%
remote met_%self.name% %actor.id%
end
~
#3240
who command~
1 c 1
wh~
if %cmd.mudcommand% == who
%send% %actor% Players who previously met in this session.
%send% %actor% -------------------------------------------
while (%counter% < 100)
makeuid mob 2+%counter%
eval player 0 + %player%
if %mob.is_pc% == 1
if %mob.varexists(met_%actor.name%)%
if %mob.is_killer%
%send% %actor% # %mob.name% (KILLER)
eval player %player% + 1
else
%send% %actor% # %mob.name%
eval player %player% + 1
end
end
end
eval counter %counter% + 1
done
if %player% < 1
%send% %actor% No player met today in this session.
elseif %player% > 1
%send% %actor% Total: %player% players.
end
else
return 0
end
~
#3241
godeye command~
0 c 100
god~
if %cmd% == godeye && %actor.varexists(god_eye)%
if !%arg%
%send% %actor% Now you know everything about nothing...
halt
end
if %arg% > 0 && %arg% < 100
%echo% fires
dg_cast 'locate object' sword
else
%send% %actor% Input a number between 1 and 99.
end
else
return 0
end
~
#3242
jajanken~
1 c 1
jaj~
if %cmd.mudcommand% == jajanken && %actor.skill(jajanken)%
if %actor.pos% == STANDING
set nen %actor.maxmana%
eval original %actor.damroll% - 1
if %actor.damroll% < 0
eval original %actor.damroll% + 1
end
eval consume (25)
if %actor.mana% >= 25
eval jajanken_loading 1
remote jajanken_loading %actor.id%
%force% %actor% say Saisho wa guu...
%send% %actor% You concentrates some amount of aura over your fist.
%echoaround% %actor% %actor.name% concentrate some amount of aura over %actor.hisher% fist.
nop %actor.mana(-%consume%)%
eval dam1 10
nop %actor.damroll(%dam1%)%
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
if %actor.mana% >= 25 && %actor.level% >= 10
eval jajanken_loading 2
remote jajanken_loading %actor.id%
%send% %actor% You concentrates an 	Wintense aura	n over your fist.
%echoaround% %actor% %actor.name% concentrate an 	Wintense aura	n over %actor.hisher% fist.
eval consume 25
nop %actor.mana(-%consume%)%
eval dam2 20
nop %actor.damroll(%dam2%)%
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
if %actor.mana% >= 25 && %actor.level% >= 15
eval jajanken_loading 3
remote jajanken_loading %actor.id%
%send% %actor% You concentrates an 	oorange aura	n over your fist.
%echoaround% %actor% %actor.name% concentrate 	oorange aura	n over %actor.hisher% fist.
nop %actor.mana(-%consume%)%
eval dam3 30
nop %actor.damroll(%dam3%)%
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
if %actor.mana% >= 25 && %actor.level% >= 20
eval jajanken_loading 4
remote jajanken_loading %actor.id%
%send% %actor% You concentrates a 	Rred aura	n over your fist.
%echoaround% %actor% %actor.name% concentrate 	Rred aura	n over %actor.hisher% fist.
nop %actor.mana(-%consume%)%
eval dam4 40
eval dammax (%dam4% + %original%)
if %dammax% > 126
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(126)%
else
nop %actor.damroll(%dam4%)%
end
wait 1 sec
if !%actor.varexists(jajanken_loading)% || %actor.pos% != standing
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
%send% %actor% The aura over your fists dissipates.
%echoaround% %actor% The aura over %actor.name%'s fists dissipates.
nop %actor.mana(%consume%)%
nop %actor.mana(%consume%)%
nop %actor.mana(%consume%)%
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
else
%send% %actor% The aura over your fists dissipates.
%echoaround% %actor% The aura over %actor.name%'s fists dissipates.
nop %actor.mana(%consume%)%
nop %actor.mana(%consume%)%
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
else
%send% %actor% The aura over your fists dissipates.
%echoaround% %actor% The aura over %actor.name%'s fists dissipates.
nop %actor.mana(%consume%)%
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
halt
end
else
%send% %actor% The aura over your fists dissipates.
%echoaround% %actor% The aura over %actor.name%'s fists dissipates.
nop %actor.damroll(-%actor.damroll%)%
nop %actor.damroll(%original%)%
if %original% < 0
eval damfix 256 + %original%
nop %actor.damroll(%damfix%)%
end
rdelete jajanken_loading %actor.id%
end
else
%send% %actor% Not enough aura to concentrate over your fist.
halt
end
else
%send% %actor% What? Now?! Impossible!
halt
end
else
return 0
end
~
#3243
bungee gum test~
0 c 100
flee~
if %cmd.mudcommand% == flee
if %actor.varexists(bungee_gummed)%
%send% %self% You tried to escape, but are gummed!
%echoaround% %self% %actor.name% tried to escape, but are gummed!
halt
else
%force% %actor% flee
halt
end
else
return 0
end
~
#3244
bung test~
0 ab 100
~
eval bungee_gummed 1
remote bungee_gummed %self.id%
~
#3245
shattered sword~
1 ab 100
~
wait 1800 sec
if %self.carried_by%
%send% %self.carried_by% %self.shortdesc% magically fix into a sword of truth.
%load% obj 65483 %self.carried_by% inv
%purge% %self%
else
%echo% %self.shortdesc% magically fix into a sword of truth.
%load% obj 65483
%purge% %self%
end
~
#3246
sword of truth detach~
1 l 100
~
detach 65383 %actor.id%
~
#3247
archangel point~
0 e 0
points at~
if %actor.varexists(player_arch)%
if %arg.cdr% == points at himself, suggesting that the center of matters is %actor.heshe%.
detach 3249 %self.id%
wait 2 sec
say 	CThen, I shall heal your body.	n
wait 2 sec
emote blows her magical breath.
wait 1 sec
dg_cast 'heal' %actor.name%
dg_cast 'exorc' %actor.name%
dg_cast 'remove poison' %actor.name%
%echo% %actor.name% is healed!
wait 2 sec
rdelete player_arch %actor.id%
say 	CNow then, farewell.	n
emote vanishes away...
%purge% %self%
end
eval i %self.room.people%
while %i%
set next %i.next_in_room%
user
if %i.vnum% != %self.vnum% && %arg.cdr% /= points at %i.name%.
detach 3249 %self.id%
wait 2 sec
say 	CThen, I shall heal %i.hisher% body.	n
wait 2 sec
emote blows her magical breath.
wait 1 sec
dg_cast 'heal' %i.name%
dg_cast 'exorc' %i.name%
dg_cast 'remove poison' %i.name%
if %i.is_pc%
%echo% %i.name% is healed!
else
%echo% %i.shortdesc% is healed!
end
wait 2 sec
rdelete player_arch %actor.id%
say 	CNow then, farewell.	n
emote vanishes away...
%purge% %self%
end
eval i %next%
done
wait 1 sec
confuse
end
end
~
#3248
shatter sword~
1 ab 100
~
if %self.carried_by%
%send% %self.carried_by% 	rYour Sword of Truth shatters!!!	n
%echoaround% %self.carried_by% %self.name%'s sword of truth shatters when tries to hit %self.carried_by.hisher% target.
%load% obj 3207 %self.carried_by% inv
%purge% %self%
else
%send% %self.worn_by% 	rYour Sword of Truth shatters!!!	n
%echoaround% %self.worn_by% 	C%self.name%'s sword of truth shatters when tries to hit %self.worn_by.hisher% target.	n
%load% obj 3207 %self.worn_by% inv
%purge% %self%
end
~
#3249
archangel heal~
0 d 1
*~
if %actor.varexists(player_arch)%
switch %speech%
case me
detach 3247 %self.id%
wait 2 sec
say 	CThen, I shall heal your body.	n
wait 2 sec
emote blows her magical breath.
wait 1 sec
dg_cast 'heal' %actor.name%
dg_cast 'exorc' %actor.name%
dg_cast 'remove poison' %actor.name%
%echo% %actor.name% is healed!
wait 2 sec
rdelete player_arch %actor.id%
say 	CNow then, farewell.	n
emote vanishes away...
%purge% %self%
case %actor.name%
detach 3247 %self.id%
wait 2 sec
say 	CThen, I shall heal your body.	n
wait 2 sec
emote blows her magical breath.
wait 1 sec
dg_cast 'heal' %actor.name%
dg_cast 'exorc' %actor.name%
dg_cast 'remove poison' %actor.name%
%echo% %actor.name% is healed!
wait 2 sec
rdelete player_arch %actor.id%
say 	CNow then, farewell.	n
emote vanishes away...
%purge% %self%
default
eval i %self.room.people%
while %i%
set next %i.next_in_room%
if %i.vnum% != %self.vnum% && %i.name% /= %speech%
detach 3247 %self.id%
wait 2 sec
say 	CThen, I shall heal %i.hisher% body.	n
wait 2 sec
emote blows her magical breath.
wait 1 sec
dg_cast 'heal' %i.name%
dg_cast 'exorc' %i.name%
dg_cast 'remove poison' %i.name%
if %i.is_pc%
%echo% %i.name% is healed!
else
%echo% %i.shortdesc% is healed!
end
wait 2 sec
rdelete player_arch %actor.id%
say 	CNow then, farewell.
emote vanishes away...
%purge% %self%
end
eval i %next%
done
wait 1 sec
say 	CJust point it.	n
halt
done
end
~
#3250
jajanken ends~
1 c 1
k~
if %cmd.mudcommand% == kill && %arg%
if %actor.varexists(jajanken_loading)%
%force% %actor% shout JANKEN... GUU!	n
rdelete jajanken_loading %actor.id%
end
%force% %actor% kill %arg%
else
return 0
end
~
#3251
booster pack~
1 c 3
unp~
if %cmd% == unpack
%send% %actor% You open %self.shortdesc%.
%echoaround% %actor% %actor.name% opened %self.shortdesc%.
set old %self.shortdesc%
eval card %random.100%
if %actor.varexists(good_luck)%
set card 100
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
%send% %actor% You unpacked %actor.inventory.shortdesc%!
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
%send% %actor% You unpacked %actor.inventory.shortdesc%!
break
default
switch %random.2%
case 1
eval i %actor.inventory%
while %i%
eval free 12073 + %random.7%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% You unpacked %actor.inventory.shortdesc%!
break
case 2
eval i %actor.inventory%
while %i%
eval free 3118 + %random.77%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% You unpacked %actor.inventory.shortdesc%!
break
default
break
done
done
end
eval card %random.100%
if %actor.varexists(good_luck)%
set card 99
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
%send% %actor% You unpacked %actor.inventory.shortdesc%!
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
%send% %actor% You unpacked %actor.inventory.shortdesc%!
break
default
switch %random.2%
case 1
eval i %actor.inventory%
while %i%
eval free 12073 + %random.7%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% You unpacked %actor.inventory.shortdesc%!
break
case 2
eval i %actor.inventory%
while %i%
eval free 3118 + %random.77%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% You unpacked %actor.inventory.shortdesc%!
break
default
break
done
done
end
eval card %random.100%
if %actor.varexists(good_luck)%
set card 99
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
%send% %actor% You unpacked %actor.inventory.shortdesc%!
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
%send% %actor% You unpacked %actor.inventory.shortdesc%!
break
default
switch %random.2%
case 1
eval i %actor.inventory%
while %i%
eval free 12073 + %random.7%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% You unpacked %actor.inventory.shortdesc%!
%purge% %self%
break
case 2
eval i %actor.inventory%
while %i%
eval free 3118 + %random.77%
%load% obj %free% %actor% inv
if %actor.inventory.vnum% == %free%
break
end
done
set new %actor.inventory.shortdesc%
%send% %actor% You unpacked %actor.inventory.shortdesc%!
%purge% %self%
halt
break
default
break
done
done
end
~
#3252
spell booster pack~
1 c 2
unp~
%send% %actor% You open %self.shortdesc%.
%echoaround% %actor% %actor.name% opened %self.shortdesc%.
eval spell 1000 + %random.40%
if %actor.varexists(good_luck)%
  eval spell 1010
end
%load% obj %spell% %actor% inv
%send% %actor% You unpacked %actor.inventory.shortdesc%!
eval spell 1000 + %random.40%
if %actor.varexists(good_luck)%
  eval spell 1026
end
%load% obj %spell% %actor% inv
%send% %actor% You unpacked %actor.inventory.shortdesc%!
eval spell 1000 + %random.40%
if %actor.varexists(good_luck)%
  eval spell 1018
end
%load% obj %spell% %actor% inv
%send% %actor% You unpacked %actor.inventory.shortdesc%!
%purge% %self%
~
#3299
Crystal Ball to Locate a Mob.~
2 q 100
~
if %actor.is_book%
%echo% Book?: %actor.is_book%
end
~
$~
