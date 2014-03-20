#!/bin/sh

#need to install network-manager xvfb fluxbox x11vnc on darwin

#killall fluxbox

#export DISPLAY=:1
#Xvfb :1 -screen 0 800x600x24 &
#fluxbox &
#sleep 10s
#x11vnc -display :1 -bg -nopw -listen localhost -xkb



# To use with remote PC
# In a terminal enter (connects to DARwIn-OP and routes session to 'localhost' of PC)
# ssh -N -T -L 5900:localhost:5900 darwin@DARWINHOST
# In a new terminal enter:
# vncviewer -encodings 'copyrect tight hextile' localhost:5900
