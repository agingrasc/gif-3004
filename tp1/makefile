transfert :
	scp cmake-build-debug/tp1 raspberry:~/bin/

debug : transfert
	ssh raspberry 'tmux new-session -d -s debug "gdbserver localhost:4242 ~/bin/tp1"'
