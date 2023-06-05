
set -x

sudo usermod -aG dialout ubuntu
sudo chmod a+rw /dev/ttyUSB0

sudo stty -F /dev/ttyUSB0 115200 cs8 ixoff -hupcl -echo  

sleep 5s

echo "F300000" > /dev/ttyUSB0



