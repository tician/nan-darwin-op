#!/bin/sh

# Install missing software
apt-get install g++ make libjpeg8-dev ncurses5-dev madplay vorbis-tools alsa-base als-util libasound2-dev openssh-blacklist openssh-blacklist-extra network-manager xvfb fluxbox x11vnc espeak

# Compile and create shortcut to TellDarwin MAX interface
cd /darwin/Linux/project/TellDarwin
make clean && make
ln -s /darwin/Linux/project/TellDarwin/TellDarwin /usr/bin/td

# apply startup sound
mv /etc/rc.local /etc/rc.local~
cp /darwin/os/rc.local /etc/rc.local

# apply shutdown sound
cp /darwin/os/darwin-shutdown-linker /etc/init.d/darwin-shutdown
ln -s /etc/init.d/darwin-shutdown /etc/rc0.d/K18shutdownscript
ln -s /etc/init.d/darwin-shutdown /etc/rc6.d/K18shutdownscript


