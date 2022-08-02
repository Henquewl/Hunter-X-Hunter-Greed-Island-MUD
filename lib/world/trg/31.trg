#3119
road trip meal gain~
1 ab 100
~
%load% obj 3103 %self.carried_by% inv
detach 3119 %self.id%
~
#3166
revert card to ticket~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
if %cmd% == gain && %actor.eq(hold).id% /= %self.id%
%force% %actor% say GAIN!
wait 1 sec
%load% obj 3066 %actor% inv
%send% %actor% Your %self.shortdesc% turns into %actor.inventory.shortdesc%.
%echoaround% %actor% %actor.name%'s %self.shortdesc% turns into %actor.inventory.shortdesc%.
%purge% %self%
else
return 0
end
~
$~
