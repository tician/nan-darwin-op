#!/bin/sh

aptitude update

aptitude upgrade

# Install missing software
aptitude install g++ make libjpeg8-dev libncurses5-dev madplay vorbis-tools alsa-base alsa-utils libasound2-dev openssh-blacklist openssh-blacklist-extra network-manager xvfb fluxbox x11vnc espeak

#exit

# Compile and create shortcut to TellDarwin MAX interface
cd /darwin/Linux/project/TellDarwin
make clean && make
ln -s /darwin/Linux/project/TellDarwin/TellDarwin /usr/bin/td

# apply startup sound
mv /etc/rc.local /etc/rc.local~
cp /darwin/os/rc.local /etc/rc.local

# apply shutdown sound
cp /darwin/os/darwin-shutdown-linker /etc/init.d/darwin-shutdown

#ln -s /etc/init.d/darwin-shutdown /etc/rc0.d/K18shutdownscript
cd /etc/rc0.d/
ln -s ../init.d/darwin-shutdown ./K18shutdownscript

#ln -s /etc/init.d/darwin-shutdown /etc/rc6.d/K18shutdownscript
cd /etc/rc6.d/
ln -s ../init.d/darwin-shutdown ./K18shutdownscript

