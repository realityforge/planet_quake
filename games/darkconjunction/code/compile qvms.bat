@echo off
cd game
call game > ..\compile_output.txt
cd ..
cd cgame
call cgame >> ..\compile_output.txt
cd ..
cd q3_ui
call q3_ui >> ..\compile_output.txt
cd ..
mkdir \quake3\conjunction\vm
del \quake3\conjunction\*.dll
del \quake3\conjunction\vm\*.qvm
copy \quake3\baseq3\vm\qagame.qvm \quake3\conjunction\vm\qagame.qvm	>> compile_output.txt
copy \quake3\baseq3\vm\q3_ui.qvm \quake3\conjunction\vm\ui.qvm		>> compile_output.txt
copy \quake3\baseq3\vm\cgame.qvm \quake3\conjunction\vm\cgame.qvm 	>> compile_output.txt
