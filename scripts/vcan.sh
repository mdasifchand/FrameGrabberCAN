#!/bin/bash

echo " please connect camera before you start this program " 

flag=false

 case $yn in
        [Yy]* )flag=true ; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac

echo " for non demo change the interface " 
echo " turninging on vcan"

sudo ip link add name vcan0 type vcan
sudo ifconfig vcan0 up


cd ../build
./main -f vcan0


