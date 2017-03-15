#18600
gain obj to card 50~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
eval hold %actor.eq(hold)%
if %cmd%==gain && %hold.id% /= %self.id%
eval card %self.vnum% + 50
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
#18601
gain card to obj 50~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
eval hold %actor.eq(hold)%
if %cmd%==gain && %hold.id% /= %self.id%
eval card %self.vnum% - 50
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
#18602
card purge timer 50~
1 fhj 100
~
set oldsdesc %self.shortdesc%
eval revert %self.vnum% - 50
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
#18603
gain obj to card +100~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
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
#18604
gain card to obj -100~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
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
#18605
gain obj to card -100~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
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
#18606
gain card to obj +100~
1 c 3
gai~
if %self.carried_by% == %actor%
%echo% You must hold something before gain it.
halt
end
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
$~
