#15000
Thief - 15015~
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
#15001
Magic User - 15032~
0 k 10
~
set skill %random.%actor.level%%
if %actor.level% < 101
  switch %skill%
    case 1
      case 2
      case 3
    break
    case 4
      dg_cast 'blast' %actor%
      nop %actor.mana(-2)%
    break
    case 5
      dg_cast 'chilltouch' %actor%
      nop %actor.mana(-2)%
    break
    case 6
      dg_cast 'burninghands' %actor%
      nop %actor.mana(-2)%
    break
    case 7
      case 8
      dg_cast 'shockinggrasp' %actor%
      nop %actor.mana(-3)%
    break
    case 9
      case 10
      case 11
      dg_cast 'lightningbolt' %actor%
      nop %actor.mana(-4)%
    break
    case 12
      dg_cast 'bungeegum' %actor%
      nop %actor.mana(-5)%
    break
    case 13
      dg_cast 'energydrain' %actor%
      nop %actor.mana(-5)%
    break
    case 14
      dg_cast 'curse' %actor%
      nop %actor.mana(-5)%
    break
    case 15
      dg_cast 'poison' %actor%
      nop %actor.mana(-5)%
    break
    case 16
      if %actor.align% > 0
        dg_cast 'dispelgood' %actor%
        nop %actor.mana(-6)%
      else
        dg_cast 'dispelevil' %actor%
        nop %actor.mana(-6)%
      end
    break
    case 17
      case 18
      dg_cast 'chainjail' %actor%
      nop %actor.mana(-7)%
    break
    case 19
      case 20
      case 21
      case 22
      dg_cast 'littleflower' %actor%
      nop %actor.mana(-8)%
    break
    default
      if %skill% > 22
        dg_cast 'auraball' %actor%
        nop %actor.mana(-8)%
      end
    break
  done
end
~
$~
