#3000
Mage Guildguard - 3024~
0 q 100
~
* Check the direction the player must go to enter the guild. 
if %direction% == south 
  * Stop them if they are not the appropriate class. 
  if %actor.class% != Magic User 
    return 0 
    %send% %actor% The guard humiliates you, and blocks your way. 
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way. 
  end 
end 
~
#3001
Cleric Guildguard - 3025~
0 q 100
~
* Check the direction the player must go to enter the guild.
if %direction% == north
  * Stop them if they are not the appropriate class.
  if %actor.class% != Cleric
    return 0
    %send% %actor% The guard humiliates you, and blocks your way.
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way.
  end
end
~
#3002
Thief Guildguard - 3026~
0 q 100
~
* Check the direction the player must go to enter the guild.
if %direction% == east
  * Stop them if they are not the appropriate class.
  if %actor.class% != Thief
    return 0
    %send% %actor% The guard humiliates you, and blocks your way.
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way.
  end
end
~
#3003
Warrior Guildguard - 3027~
0 q 100
~
* Check the direction the player must go to enter the guild.
if %direction% == east
  * Stop them if they are not the appropriate class.
  if %actor.class% != Warrior
    return 0
    %send% %actor% The guard humiliates you, and blocks your way.
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way.
  end
end
~
#3004
Dump - 3030~
2 h 100
~
%send% %actor% You are awarded for outstanding performance.
%echoaround% %actor% %actor.name% has been awarded for being a good citizen.
eval value %object.cost% / 10
if %value% > 50
  set value 50
elseif %value% < 1
  set value 1
end
if %actor.level% < 3
  nop %actor.exp(%value%)%
else
  nop %actor.gold(%value%)%
end
return 0
~
#3005
Stock Thief~
0 b 10
~
set actor %random.char%
if %actor%
  if %actor.is_pc% && %actor.gold%
    %send% %actor% You discover that %self.name% has %self.hisher% hands in your wallet.
    %echoaround% %actor% %self.name% tries to steal gold from %actor.name%.
    eval coins %actor.gold% * %random.10% / 100
    nop %actor.gold(-%coins%)%
    nop %self.gold(%coins%)%
  end
end
~
#3006
Stock Snake~
0 k 10
~
%send% %actor% %self.name% bites you!
%echoaround% %actor% %self.name% bites %actor.name%.
dg_cast 'poison' %actor%
~
#3007
Stock Magic User~
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
#3008
Near Death Trap~
2 g 100
~
* By Rumble of The Builder Academy    tbamud.com 9091
* Near Death Trap stuns actor
set stunned %actor.hitp% 
%damage% %actor% %stunned%
%send% %actor% You are on the brink of life and death.
%send% %actor% The Gods must favor you this day.
~
#3009
Stock Cityguard - 3059, 60, 67~
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
#3010
Stock Fido - 3062, 3066~
0 b 100
~
set inroom %self.room%
set item %inroom.contents%
while %item%
  * Target the next item in room. In case it is devoured.
  set next_item %item.next_in_list%
  * Check for a corpse. Corpse on TBA is vnum 65535. Stock is -1.
  if %item.vnum(65535)%
    emote savagely devours a corpse.
    %purge% %item%
    halt
  end
  set item %next_item%
  * Loop back
done
~
#3011
Stock Janitor - 3061, 3068~
0 b 100
~
eval inroom %self.room%
eval item %inroom.contents%
while %item%
  * Target the next item in room. In case it is picked up.
  set next_item %item.next_in_list%
* TODO: if %item.wearflag(take)% 
  * Check for fountains and expensive items.
  if %item.type% != FOUNTAIN && %item.cost% <= 15
    take %item.name%
  end
  set item %next_item%
  * Loop back
done
~
#3012
Newbie Tour Guide~
0 e 0
has entered the game.~
%echo% This trigger commandlist is not complete!
~
#3013
Newbie Tour Guide Loader~
0 e 0
has entered the game.~
* By Rumble of The Builder Academy    tbamud.com 9091
* Num Arg 0 means the argument has to match exactly. So trig will only fire off:
* "has entered game." and not "has" or "entered" etc. (that would be num arg 1).
* Figure out what vnum the mob is in so we can use zoneecho.
eval inroom %self.room%
%zoneecho% %inroom.vnum% %self.name% shouts, 'Welcome, %actor.name%!'
~
#3014
Teleporter~
1 c 3
teleport~
* By Rumble and Jamie Nelson of The Builder Academy    tbamud.com 9091
%send% %actor% You attempt to manipulate space and time.
%echoaround% %actor% %actor.name% attempts to manipulate space and time.
wait 1 sec
set sanctus 100
set jade 400
set newbie 500
set sea 600
set camelot 775
set nuclear 1800
set spider 1999
set arena 2000
set tower 2200
set memlin 2798
set mudschool 2800
set midgaard 3001
set capital 3702
set haven 3998
set chasm 4200
set arctic 4396
set Orc 4401
set monastery 4512
set ant 4600
set zodiac 5701
set grave 7401
set zamba 7500
set gidean 7801
set glumgold 8301
set duke 8660
set oasis 9000
set domiae 9603
set northern 10004
set south 10101
set dbz 10301
set orchan 10401
set elcardo 10604
set iuel 10701
set omega 11501
set torres 11701
set dollhouse 11899
set hannah 12500
set maze 13001
set wyvern 14000
set caves 16999
set cardinal 17501
set circus 18700
set western 20001
set sapphire 20101
set kitchen 22001
set terringham 23200
set dragon 23300
set school 23400
set mines 23500
set aldin 23601
set crystal 23875
set pass 23901
set maura 24000
set enterprise 24100
set new 24200
set valley 24300
set prison 24457
set nether 24500
set yard 24700
set elven 24801
set jedi 24901
set dragonspyre 25000
set ape 25100
set vampyre 25200
set windmill 25300
set village 25400
set shipwreck 25516
set keep 25645
set jareth 25705
set light 25800
set mansion 25907
set grasslands 26000
set igor's 26100
set forest 26201
set farmlands 26300
set banshide 26400
set beach 26500
set ankou 26600
set vice 26728
set desert 26900
set wasteland 27001
set sundhaven 27119
set station 27300
set smurfville 27400
set sparta 27501
set shire 27700
set oceania 27800
set notre 27900
set motherboard 28000
set khanjar 28100
set kerjim 28200
set haunted 28300
set ghenna 28400
set hell 28601
set goblin 28700
set galaxy 28801
set werith's 28900
set lizard 29000
set black 29100
set kerofk 29202
set trade 29400
set jungle 29500
set froboz 29600
set desire 29801
set cathedral 29900
set ancalador 30000
set campus 30100
set bull 30401
set chessboard 30537
set tree 30600
set castle 30700
set baron 30800
set westlawn 30900
set graye 31003
set teeth 31100
set leper 31200
set altar 31400
set mcgintey 31500
set wharf 31700
set dock 31801
set yllnthad 31900
set bay 32200
set pale 32300
set army 32400
set revelry 32500
set perimeter 32600
set asylum 34501
set ultima 55685
set tarot 21101
if !%arg%
  *they didnt type a location
  set fail 1
else
  *take the first word they type after the teleport command
  *compare it to a variable above
  eval loc %%%arg.car%%%
  if !%loc%
    *they typed an invalid location
    set fail 1
  end
end
if %fail%
  %send% %actor% You fail.
  %echoaround% %actor% %actor.name% fails.
  halt
end
%echoaround% %actor% %actor.name% seems successful as %actor.heshe% steps into another realm.
%teleport% %actor% %loc%
%force% %actor% look
%echoaround% %actor% %actor.name% steps out of space and time.
~
#3015
Teleporter Recall and Return~
1 c 7
re~
if %cmd% == recall && !%actor.is_sleeping%
eval teleporter_return_room %actor.room.vnum%
remote  teleporter_return_room %actor.id%
%send% %actor% You recall to safety.
%echoaround% %actor% %actor.name% recalls.
%teleport% %actor% 3001
%force% %actor% look
%echoaround% %actor% %actor.name% appears in the room.
elseif %cmd% == return
%send% %actor% You return to your previous location.
%echoaround% %actor% %actor.name% teleports out of the room.
%teleport% %actor% %actor.teleporter_return_room%
%force% %actor% look
%echoaround% %actor% %actor.name% appears in the room.
else
return 0
end
~
#3016
Kind Soul Gives Newbie Equipment~
0 g 100
~
if %direction% == south
if !%actor.eq(*)%
%load%obj 3037 %actor% light
%load%obj 3082 %actor% neck1
%load%obj 3082 %actor% neck2
%load%obj 3043 %actor% body
%load%obj 3076 %actor% head
%load%obj 3081 %actor% legs
%load%obj 3084 %actor% feet
%load%obj 3071 %actor% hands
%load%obj 3086 %actor% arms
%load%obj 3042 %actor% shield
%load%obj 3087 %actor% about
%load%obj 3088 %actor% waist
%load%obj 3089 %actor% rwrist
%load%obj 3089 %actor% lwrist
%load%obj 3023 %actor% wield
end
end
~
#3017
Mortal Greet~
2 s 100
~
wait 1 sec
if %actor.level% < 2
%load%obj 3037 %actor% light
%load%obj 3083 %actor% rfinger
%load%obj 3083 %actor% lfinger
%load%obj 3082 %actor% neck1
%load%obj 3082 %actor% neck2
%load%obj 3043 %actor% body
%load%obj 3076 %actor% head
%load%obj 3081 %actor% legs
%load%obj 3084 %actor% feet
%load%obj 3071 %actor% hands
%load%obj 3086 %actor% arms
%load%obj 3042 %actor% shield
%load%obj 3087 %actor% about
%load%obj 3088 %actor% waist
%load%obj 3089 %actor% rwrist
%load%obj 3089 %actor% lwrist
%load%obj 3023 %actor% wield
%load%obj 18606 %actor% hold
%purge% %actor.inventory(3037)%
%purge% %actor.inventory(3083)%
%purge% %actor.inventory(3083)%
%purge% %actor.inventory(3082)%
%purge% %actor.inventory(3082)%
%purge% %actor.inventory(3043)%
%purge% %actor.inventory(3076)%
%purge% %actor.inventory(3081)%
%purge% %actor.inventory(3084)%
%purge% %actor.inventory(3071)%
%purge% %actor.inventory(3086)%
%purge% %actor.inventory(3042)%
%purge% %actor.inventory(3087)%
%purge% %actor.inventory(3088)%
%purge% %actor.inventory(3089)%
%purge% %actor.inventory(3089)%
%purge% %actor.inventory(3023)%
%purge% %actor.inventory(18606)%
end
if %actor.level% =< 10
%purge% %actor.inventory(18)%
%load%obj 18 %actor% inv
end
~
#3018
Kind Soul Gives Newbie Equipment~
0 ab 100
~
if !%actor.eq(*)%
say get some clothes on! Here, I will help.
%load%obj 3037 %actor% light
%load%obj 3083 %actor% rfinger
%load%obj 3083 %actor% lfinger
%load%obj 3082 %actor% neck1
%load%obj 3082 %actor% neck2
%load%obj 3040 %actor% body
%load%obj 3076 %actor% head
%load%obj 3080 %actor% legs
%load%obj 3084 %actor% feet
%load%obj 3071 %actor% hands
%load%obj 3086 %actor% arms
%load%obj 3042 %actor% shield
%load%obj 3087 %actor% about
%load%obj 3088 %actor% waist
%load%obj 3089 %actor% rwrist
%load%obj 3089 %actor% lwrist
%load%obj 3021 %actor% wield
%load%obj 3055 %actor% hold
%load%obj 18 %actor% inv
%load%obj 3010 %actor% inv
%load%obj 3014 %actor% inv
%load%obj 3018 %actor% inv
%load%obj 3104 %actor% inv
say There, take some coins and buy something for you.
give 1000 coins %actor.name%
halt
end
if !%actor.eq(light)%
Say you really shouldn't be wandering these parts without a light source %actor.name%.
shake
%load%obj 3037
give candle %actor.name%
halt
end
~
#3019
no get level 10~
1 g 100
~
if %actor.level% < 10 && !%actor.inventory(18)%
return 1
else
%send% %actor% %self.shortdesc% turns to dust when you touch it...
%purge% %self%
return 0
end
~
#3020
Temple heal~
2 b 100
~
set actor %random.char%
if %actor.hunger% == 0 && %actor.thirst% == 0
if %actor.hitp% < %actor.maxhitp%
eval h %actor.maxhitp% / 50
%damage% %actor% -%h%
end
if %actor.mana% < %actor.maxmana%
eval m %actor.maxmana% / 50
nop %actor.mana(%m%)%
if %actor.mana% > %actor.maxmana%
eval m %actor.maxmana% - %actor.mana%
nop %actor.mana(%m%)%
end
end
if %actor.move% < %actor.maxmove%
eval v %actor.level% / 4
nop %actor.move(%v%)%
if %actor.move% > %actor.maxmove%
eval v %actor.maxmove% - %actor.move%
nop %actor.move(%v%)%
end
end
halt
end
if %actor.hunger% == 0 && %actor.thirst% > 0
if %actor.hitp% < %actor.maxhitp%
eval h %actor.maxhitp% / 20
%damage% %actor% -%h%
end
if %actor.mana% < %actor.maxmana%
eval m %actor.maxmana% / 50
nop %actor.mana(%m%)%
if %actor.mana% > %actor.maxmana%
eval m %actor.maxmana% - %actor.mana%
nop %actor.mana(%m%)%
end
end
if %actor.move% < %actor.maxmove%
eval v %actor.level% / 2
nop %actor.move(%v%)%
if %actor.move% > %actor.maxmove%
eval v %actor.maxmove% - %actor.move%
nop %actor.move(%v%)%
end
end
halt
end
if %actor.thirst% == 0 && %actor.hunger% > 0
if %actor.hitp% < %actor.maxhitp%
eval h %actor.maxhitp% / 50
%damage% %actor% -%h%
end
if %actor.mana% < %actor.maxmana%
eval m %actor.maxmana% / 20
nop %actor.mana(%m%)%
if %actor.mana% > %actor.maxmana%
eval m %actor.maxmana% - %actor.mana%
nop %actor.mana(%m%)%
end
end
if %actor.move% < %actor.maxmove%
eval v %actor.level% / 2
nop %actor.move(%v%)%
if %actor.move% > %actor.maxmove%
eval v %actor.maxmove% - %actor.move%
nop %actor.move(%v%)%
end
end
halt
end
if %actor.hitp% < %actor.maxhitp%
eval h %actor.maxhitp% / 20
%damage% %actor% -%h%
end
if %actor.mana% < %actor.maxmana%
eval m %actor.maxmana% / 20
nop %actor.mana(%m%)%
if %actor.mana% > %actor.maxmana%
nop %actor.mana(%actor.maxmana%)%
end
end
if %actor.move% < %actor.maxmove%
set v %actor.level%
nop %actor.move(%v%)%
if %actor.move% > %actor.maxmove%
eval v %actor.maxmove% - %actor.move%
nop %actor.move(%v%)%
end
end
~
#3021
master heal players~
0 ab 100
~
set player %random.char%
dg_cast 'nen cure' %player.name%
dg_cast 'nen cure' %player.name%
dg_cast 'heal' %player.name%
say Keep going %player.name%, or I'll make you do 1000 pushups!
~
#3030
Defensive spell stock~
2 f 100
~
set actor %self.people% 
while %actor% 
  set tmp_target %actor.next_in_room% 
  if !%actor.is_pc% && %actor.vnum(3036)%
    %load% obj 1003 %actor% inv
    %load% obj 1003 %actor% inv
    %load% obj 1003 %actor% inv
    %load% obj 1003 %actor% inv
    %load% obj 1004 %actor% inv
    if %random.5% == 5
      %load% obj 1004 %actor% inv
    end
    %load% obj 1019 %actor% inv
    %load% obj 1019 %actor% inv
    %load% obj 1025 %actor% inv
    %load% obj 1025 %actor% inv
	if %random.5% == 5
    %load% obj 1026 %actor% inv
	end
    if %random.10% == 10
      %load% obj 1035 %actor% inv
    end
    if %random.10% == 10
      %load% obj 1036 %actor% inv
    end
  end 
  set actor %tmp_target% 
done 
~
#3031
Regular spell stock~
2 f 100
~
set actor %self.people% 
while %actor% 
  set tmp_target %actor.next_in_room% 
  if !%actor.is_pc% && %actor.vnum(3000)%
    %load% obj 1001 %actor% inv
    %load% obj 1001 %actor% inv
    %load% obj 1002 %actor% inv
    if %random.2% == 2
      %load% obj 1002 %actor% inv
    end
    if %random.2% == 2
      %load% obj 1005 %actor% inv
    end
    %load% obj 1009 %actor% inv
    %load% obj 1009 %actor% inv
    %load% obj 1009 %actor% inv
    if %random.10% > 2
      %load% obj 1009 %actor% inv
    end
    if %random.5% == 5
      %load% obj 1010 %actor% inv
    end
    if %random.10% > 3
      %load% obj 1011 %actor% inv
    end
    %load% obj 1012 %actor% inv
    if %random.10% > 6
      %load% obj 1012 %actor% inv
    end
    if %random.100% > 35
      %load% obj 1013 %actor% inv
    end
    if %random.10% > 7
      %load% obj 1014 %actor% inv
    end
    if %random.10% > 3
      %load% obj 1015 %actor% inv
    end
    %load% obj 1016 %actor% inv
    %load% obj 1016 %actor% inv
    %load% obj 1017 %actor% inv
    %load% obj 1017 %actor% inv
    if %random.10% > 6
      %load% obj 1020 %actor% inv
    end
    if %random.10% > 2
      %load% obj 1024 %actor% inv
    end
    %load% obj 1030 %actor% inv
    if %random.5% == 5
      %load% obj 1030 %actor% inv
    end
	%load% obj 1031 %actor% inv
	%load% obj 1031 %actor% inv
	%load% obj 1031 %actor% inv
	%load% obj 1031 %actor% inv
    %load% obj 1032 %actor% inv
    %load% obj 1032 %actor% inv
    %load% obj 1032 %actor% inv
    if %random.2% == 2
      %load% obj 1032 %actor% inv
    end
    if %random.100% > 45
      %load% obj 1034 %actor% inv
    end
    %load% obj 1037 %actor% inv
    if %random.10% > 3
      %load% obj 1037 %actor% inv
    end
    %load% obj 1038 %actor% inv
    %load% obj 1038 %actor% inv
    %load% obj 1038 %actor% inv
    if %random.2% == 2
      %load% obj 1038 %actor% inv
    end
    %load% obj 1039 %actor% inv
    if %random.10% > 7
      %load% obj 1039 %actor% inv
    end
    %load% obj 1040 %actor% inv
    %load% obj 1040 %actor% inv
  end 
  set actor %tmp_target% 
done 
~
#3032
Attack spell stock~
2 f 100
~
set actor %self.people% 
while %actor% 
  set tmp_target %actor.next_in_room% 
  if !%actor.is_pc% && %actor.vnum(3035)%
    %load% obj 1006 %actor% inv
    if %random.10% > 3
      %load% obj 1006 %actor% inv
    end
    if %random.2% == 2
      %load% obj 1007 %actor% inv
    end
    %load% obj 1008 %actor% inv
    if %random.4% == 4
      %load% obj 1018 %actor% inv
    end
    if %random.10% > 7
      %load% obj 1021 %actor% inv
    end
    if %random.10% != 1
      %load% obj 1027 %actor% inv
    end
    %load% obj 1028 %actor% inv
    if %random.4% == 4
      %load% obj 1029 %actor% inv
    end
    if %random.2% == 2
      %load% obj 1033 %actor% inv
    end
  end 
  set actor %tmp_target% 
done 
~
#3033
wizard spell store~
0 ab 100
~
switch %random.99%
case 99
say I am too tired to give explanations about cards...
wait 3 sec
say Just visit http://hunterxhunter.wikia.com/wiki/Greed_Island_Card_Lists
wait 3 sec
say ... and spare me of your questions.
break
default
break
done
~
#3034
new shipment~
2 f 100
~
wait 13 sec
%zoneecho% %self.vnum% A flying balloon says, 'ATTENTION!! A new shipment of spell cards has arrived!'
~
#3035
wizard spell store 2~
0 ab 100
~
set actor %random.char%
if %actor.is_pc% == 1
  switch %random.20%
    case 1
      say You come to the right place!
    break
    case 2
      say Welcome to best shop EVER!
    break
    case 3
      smile %actor.name%
    break
    case 4
      grin %actor.name%
    break
    case 5
      curtsey %actor.name%
    break
    case 6
      shake %actor.name%
    break
    case 7
      hug %actor.name%
      say Attack spells... attack spells everywhere!
    break
    case 8
      say Rob is the best attack spell EVER! Make someone cry RIGHT NOW!
    break
    case 9
      say Levy is the best attack spell EVER! Make other people cry RIGHT NOW!
    break
    case 10
      say Did you would be interested in making a card from our store?
      wait 2 sec
      say What I'm saying?! WE sell cards!
      laugh
    break
    case 11
      handraise %actor.name%
    break
    case 12
      handshake %actor.name%
    break
    case 13
      salute %actor.name%
    break
    case 14
      salute
    break
    case 15
      cough
    break
    case 16
      sneeze
    break
    case 17
      yawn
    break
    case 18
      stretch
    break
    case 19
      cheers %actor.name%
    break
    case 20
      comb
    break
    default
    break
  done
end
~
#3036
wizard spell store 3~
0 ab 100
~
switch %random.60%
case 10
say Defensive Wall spell protect against one attack spell about to be casted on you.
break
case 20
say Reflection spell protect and reflect back an attack spell about to be casted on you.
break
case 30
say Castle Gate spell protect against one regular spell about to be casted on you.
break
case 40
say Blackout Curtain spell protects you against "Peek" or "Fluroscopy" spell once.
break
case 50
say Holy Water spell protects you against attack spells 10 times.
break
case 60
say Prison spell protects yours restricted cards against stealing and destruction spells.
break
default
break
done
~
#3037
travel to exit~
0 j 100
~
if %object.vnum% == 3066
%purge% %object%
wait 1 sec
say Hooya! Dock chief gave you this?
wait 2 sec
say Well, thats not my business...
wait 3 sec
say Come on fellow, it is time to set sail!
wait 1 sec
%teleport% %actor% 1406
%force% %actor% look
elseif %object.vnum% == 3166
wait 1 sec
say I like play cards boy but, I can do nothing with that crap.
drop %object.name%
else
wait 1 sec
say No rats in my ship, get off!... and take back that crap!
drop %object.name%
end
~
#3053
visited masadora tag~
2 q 100
~
if %actor.is_pc%
rdelete visited_masadora %actor.id%
eval visited_masadora %self.vnum%
remote visited_masadora %actor.id%
end
~
#3066
revert ticket to card~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
if %cmd% == gain && %actor.eq(hold).id% /= %self.id%
%force% %actor% say GAIN!
wait 1 sec
%load%obj 3166 %actor% inv
%send% %actor% Your %self.shortdesc% turns into %actor.inventory.shortdesc%.
%echoaround% %actor% %actor.name%'s %self.shortdesc% turns into %actor.inventory.shortdesc%.
%purge% %self%
else
return 0
end
~
#3090
spyglass effect~
1 c 3
spy~
if %self.carried_by% == %actor%
%echo% You see deep far into your pocket, maybe holding it will be more efficient.
halt
end
if %cmd% == spy
if !%arg%
%send% %actor% Ok but, what direction!?!
halt
end
switch %arg.car%
case n
set way north
break
case e
set way east
break
case s
set way south
break
case w
set way west
break
case u
set way up
break
case d
set way down
break
case ne
set way northeast
break
case nw
set way northwest
break
case se
set way southeast
break
case sw
set way southwest
break
default
%send% %actor% Usage: spy <n><e><s><w><u><d><nw><sw><se><ne>
halt
break
done
set pr %actor.room%
set direction %way%
eval result %%pr.%direction%%%
if !%result%
%send% %actor% There is nothing in that direction.
halt
end
set vloc %way%(vnum)
eval floc %%pr.%vloc%%%
set prevloc %actor.room.vnum%
switch %result%
case DOOR
set vloc %way%(vnum)
eval floc %%pr.%vloc%%%
%send% %actor% Spying %way%...
%echoaround% %actor% %actor.name% gazes to %way% with a spyglass.
%teleport% %actor% %floc%
%force% %actor% look
%teleport% %actor% %prevloc%
break
case DOOR CLOSED
%force% %actor% look %way%
break
case DOOR CLOSED LOCKED
%force% %actor% look %way%
break
case DOOR UNDEFINED
%force% %actor% look %way%
break
case DOOR CLOSED UNDEFINED
%force% %actor% look %way%
break
case DOOR CLOSED LOCKED UNDEFINED
%force% %actor% look %way%
break
default
set vloc %way%(vnum)
eval floc %%pr.%vloc%%%
%send% %actor% Spying %way%...
%echoaround% %actor% %actor.name% gazes to %way% with a spyglass.
%teleport% %actor% %floc%
%force% %actor% look
%teleport% %actor% %prevloc%
break
done
else
return 0
end
~
#3098
test2~
2 c 100
test~
%send% %actor% Players who previously met in this session.
%send% %actor% -------------------------------------------
while (%counter% < 100)
makeuid mob 2+%counter%
eval player 0 + %player%
if %mob.is_pc% == 1
if %mob.varexists(met_%actor.name%)%
%send% %actor% # %mob.name%
eval player %player% + 1
end
end
eval counter %counter% + 1
done
if %player% < 1
%send% %actor% No player met today in this session.
elseif %player% > 1
%send% %actor% Total: %player% players.
end
~
#3099
Test~
2 b 1
~
%zoneecho% 3001 You hear a loud --=BOOM=--,
~
$~
