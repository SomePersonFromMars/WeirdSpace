gnome-terminal --tab -- bash -c "bash --rcfile <(echo '. ~/.bashrc; cd ~/Dev/WeirdSpace/game; vim -S Session.vim')"
gnome-terminal --tab -- bash -c "bash --rcfile <(echo '. ~/.bashrc; cd ~/Dev/WeirdSpace/build/game')"
gnome-terminal --tab -- bash -c "bash --rcfile <(echo '. ~/.bashrc; cd ~/Dev/WeirdSpace/game; git status')"
