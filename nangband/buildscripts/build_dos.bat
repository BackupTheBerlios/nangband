mkdir ..\angband-291
mkdir ..\angband-291\lib
mkdir ..\angband-291\lib\apex
mkdir ..\angband-291\lib\bone
mkdir ..\angband-291\lib\data
mkdir ..\angband-291\lib\edit
mkdir ..\angband-291\lib\file
mkdir ..\angband-291\lib\help
mkdir ..\angband-291\lib\info
mkdir ..\angband-291\lib\pref
mkdir ..\angband-291\lib\save
mkdir ..\angband-291\lib\user
mkdir ..\angband-291\lib\xtra
mkdir ..\angband-291\lib\xtra\font
mkdir ..\angband-291\lib\xtra\graf
mkdir ..\angband-291\lib\xtra\music
mkdir ..\angband-291\lib\xtra\sound

type lib\apex\delete.me >..\angband-291\lib\apex\delete.me
type lib\bone\delete.me >..\angband-291\lib\bone\delete.me
type lib\data\delete.me >..\angband-291\lib\data\delete.me
type lib\info\delete.me >..\angband-291\lib\info\delete.me
type lib\pref\delete.me >..\angband-291\lib\pref\delete.me
type lib\save\delete.me >..\angband-291\lib\save\delete.me
type lib\xtra\music\delete.me >..\angband-291\lib\xtra\music\delete.me

copy angband.exe ..\angband-291
copy readme.txt ..\angband-291
copy changes.txt ..\angband-291
copy cwsdpmi.exe ..\angband-291
copy angdos.cfg ..\angband-291


copy lib\edit\*.txt ..\angband-291\lib\edit

copy lib\file\*.txt ..\angband-291\lib\file

copy lib\help\*.txt ..\angband-291\lib\help
copy lib\help\*.hlp ..\angband-291\lib\help

copy lib\user\font.prf ..\angband-291\lib\user
copy lib\user\font-dos.prf ..\angband-291\lib\user
copy lib\user\font-ibm.prf ..\angband-291\lib\user
copy lib\user\font-xxx.prf ..\angband-291\lib\user
copy lib\user\graf.prf ..\angband-291\lib\user
copy lib\user\graf-ibm.prf ..\angband-291\lib\user
copy lib\user\graf-new.prf ..\angband-291\lib\user
copy lib\user\graf-win.prf ..\angband-291\lib\user
copy lib\user\graf-xxx.prf ..\angband-291\lib\user
copy lib\user\pref.prf ..\angband-291\lib\user
copy lib\user\pref-win.prf ..\angband-291\lib\user
copy lib\user\user.prf ..\angband-291\lib\user
copy lib\user\xtra-new.prf ..\angband-291\lib\user
copy lib\user\xtra-xxx.prf ..\angband-291\lib\user

copy lib\xtra\font\*.fnt ..\angband-291\lib\xtra\font

copy lib\xtra\graf\8x8.bmp ..\angband-291\lib\xtra\graf
copy lib\xtra\graf\16x16.bmp ..\angband-291\lib\xtra\graf
copy lib\xtra\graf\backgrnd.bmp ..\angband-291\lib\xtra\graf

copy lib\xtra\sound\sound.cfg ..\angband-291\lib\xtra\sound
copy lib\xtra\sound\*.wav ..\angband-291\lib\xtra\sound

upx -9 ..\angband-291\angband.exe

