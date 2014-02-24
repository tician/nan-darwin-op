#!/bin/sh

apt-get update
apt-get upgrade

# Install missing software
apt-get install --no-install-recommends g++ make libjpeg8-dev libncurses5-dev mpg321 vorbis-tools libasound2-dev openssh-blacklist openssh-blacklist-extra network-manager-gnome xvfb fluxbox x11vnc espeak pulseaudio bc git-core rsync
# alsa-base alsa-utils

#exit

echo "@$USER    hard    rtpio    32" >> /etc/security/limits.conf
echo "@$USER    soft    rtpio    32" >> /etc/security/limits.conf
#echo "@$USER    hard    nice     0" >> /etc/security/limits.conf
#echo "@$USER    soft    nice     0" >> /etc/security/limits.conf

# Keep the nosy out of the sudo home
chmod 0750 /home/$USER

# Create non-superuser to avoid future software issues.
adduser darwin

# Create darwin-system folder
mkdir /darwin

# Install TellDarwin and DARwIn-OP framework and most up-to-date configs
git clone git://github.com/tician/nan-darwin-op.git /home/darwin/src/nan-darwin-op
rsync -av /home/darwin/src/nan-darwin-op/ /darwin
chown -R darwin:darwin /home/darwin

chown -R darwin:darwin /darwin

# Update .bashrc with color and firmware message
cp -f /darwin/os/configs/.bashrc /home/darwin/.bashrc

# Enable non-superusers to use poweroff and reboot
groupadd shutdown
usermod -a -G shutdown darwin
# Enable non-superusers to create new network connections
groupadd nmusers
usermod -a -G nmusers darwin
# Create groups first, so we don't break sudo again...
cp /darwin/os/configs/sudoermods /etc/sudoers.d/sudoermods
chmod 0440 /etc/sudoers.d/sudoermods

# Try to enable non-superusers to run any CM730 program without sudo
usermod -a -G dialout darwin
usermod -a -G video darwin

# Compile and create shortcut to TellDarwin MAX interface
cd /darwin/Linux/project/TellDarwin
make clean && make
ln -s /darwin/Linux/project/TellDarwin/TellDarwin /usr/bin/td

# Enable startup sound
mv /etc/rc.local /etc/rc.local~
cp /darwin/os/configs/rc.local /etc/rc.local

# Enable shutdown sound
cp /darwin/os/configs/darwin-shutdown-linker /etc/init.d/darwin-shutdown
ln -s /etc/init.d/darwin-shutdown /etc/rc0.d/K18shutdownscript
ln -s /etc/init.d/darwin-shutdown /etc/rc6.d/K18shutdownscript

# Augment motd with uptime
cp /darwin/os/configs/05-motd-tail-updater /etc/update-motd.d/05-motd-tail-updater


# Remind user to make sure speaker is not muted and has tolerable volume
echo "Setup finished.  Don't forget to use alsamixer to configure volume."
