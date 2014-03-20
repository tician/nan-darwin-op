#!/bin/sh

SUDO_USER=zzr

apt-get update
apt-get upgrade

# Install missing software
apt-get install --no-install-recommends g++ make libjpeg8-dev libncurses5-dev mpg321 vorbis-tools libasound2-dev alsa-base alsa-utils openssh-blacklist openssh-blacklist-extra network-manager-gnome xvfb fluxbox x11vnc espeak bc git-core rsync vnc4server xserver-xorg xterm eterm
# pulseaudio

# Keep the nosy out of the SUDO_USER home
chmod 0750 /home/$SUDO_USER

# Update .bashrc with color and firmware message
cp -f /darwin/os/configs/.bashrc /home/$SUDO_USER/.bashrc
chown $SUDO_USER:$SUDO_USER /home/$SUDO_USER/.bashrc

# Create darwin-system folder
mkdir /darwin
mkdir /home/$SUDO_USER/src

# Install TellDarwin and DARwIn-OP framework and most up-to-date configs
git clone git://github.com/tician/nan-darwin-op.git /home/$SUDO_USER/src/nan-darwin-op
chown -R $SUDO_USER:$SUDO_USER /home/$SUDO_USER/src
#chmod -R go+rw /home/$SUDO_USER/src
rsync -av /home/$SUDO_USER/src/nan-darwin-op/ /darwin



# Create udev rule to assign the CM-730 a persistent name "ttyCM730"
#CM730_FTDI_ID=$(lsusb -v -d 0403:6001 | grep iSerial | sed 's/ iSerial * [0-9] * //g')
CM730_FTDI_ID=$(udevadm info -a -n /dev/ttyUSB0 | grep '{serial}' | head -n1 | sed 's/ //g')
CM730_UDEV=$(echo "SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"0403\", ATTRS{idProduct}==\"6001\", $CM730_FTDI_ID, SYMLINK+=\"ttyCM730\"")
echo "$CM730_UDEV" >> /etc/udev/rules.d/99-usb-serial.rules


# Enable darwin and SUDO_USER to use real-time threads
LIMITS_CONF=$(cat /etc/security/limits.conf | sed 's/# End of file//g')
echo "$LIMITS_CONF" > /etc/security/limits.conf
echo "@darwin    hard    rtprio    unlimited" >> /etc/security/limits.conf
echo "@$SUDO_USER    hard    rtprio    unlimited" >> /etc/security/limits.conf
echo "@darwin    soft    rtprio    32" >> /etc/security/limits.conf
echo "@$SUDO_USER    soft    rtprio    32" >> /etc/security/limits.conf

echo "@darwin    hard    nice     0" >> /etc/security/limits.conf
echo "@$SUDO_USER    hard    nice     0" >> /etc/security/limits.conf
echo "@darwin    soft    nice     0" >> /etc/security/limits.conf
echo "@$SUDO_USER    soft    nice     0" >> /etc/security/limits.conf

echo "# End of file\n" >> /etc/security/limits.conf



# Create non-superuser to avoid future software issues.
adduser darwin

# Update .bashrc with color and firmware message
cp -f /darwin/os/configs/.bashrc /home/darwin/.bashrc
chown darwin:darwin /home/darwin/.bashrc

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



# Enable startup sound
mv /etc/rc.local /etc/rc.local~
cp /darwin/os/configs/rc.local /etc/rc.local
chmod 0755 /etc/rc.local

# Enable shutdown sound
cp /darwin/os/configs/darwin-shutdown-linker /etc/init.d/darwin-shutdown
chmod 0755 /etc/init.d/darwin-shutdown

ln -s /etc/init.d/darwin-shutdown /etc/rc0.d/K18shutdownscript
ln -s /etc/init.d/darwin-shutdown /etc/rc6.d/K18shutdownscript

# Augment motd with uptime
cp /darwin/os/configs/05-motd-tail-updater /etc/update-motd.d/05-motd-tail-updater
chmod 0755 /etc/update-motd.d/05-motd-tail-updater

chown -R darwin:darwin /darwin
#chmod -R go+rw /darwin



sudo usermod -a -G audio $SUDO_USER
sudo usermod -a -G audio darwin


cp /darwin/os/configs/vncserver /etc/


# Compile DARwIn-OP demo
#cd /darwin/Linux/project/demo
#make distclean && make
# Compile and create shortcut to TellDarwin MAX interface
#cd /darwin/Linux/project/TellDarwin
#make clean && make
#ln -s /darwin/Linux/project/TellDarwin/TellDarwin /usr/bin/td

# Remind user to make sure speaker is not muted and has tolerable volume
echo "Setup finished.  Don't forget to use alsamixer to configure volume."
echo "If you get an error about no mixer, add users to the audio group with:"
echo " sudo usermod -a -G audio $SUDO_USER"
echo " sudo usermod -a -G audio darwin"


