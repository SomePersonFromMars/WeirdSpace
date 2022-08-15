if [ -z $1 ]
then
	# subproject=game
	subproject=tools/generator_playground
	# subproject=tools/obj_dump
else
	subproject=$1
fi

gnome-terminal --tab -- bash -c "bash --rcfile <(echo '. ~/.bashrc; cd ~/Dev/WeirdSpace/${subproject}; vim -S')"
gnome-terminal --tab -- bash -c "bash --rcfile <(echo '. ~/.bashrc; cd ~/Dev/WeirdSpace/build/${subproject}')"
gnome-terminal --tab -- bash -c "bash --rcfile <(echo '. ~/.bashrc; cd ~/Dev/WeirdSpace/${subproject}; git status')"
