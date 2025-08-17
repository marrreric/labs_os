#!/bin/bash
if getcap lab9_1 &>/dev/null; then
    echo "Сброс возможностей"
    sudo setcap -r lab9_1
fi
echo -e "\nБез возможностей"
./lab9_1
echo -e "\nУстанавливаю cap_net_bind_service"
sudo setcap cap_net_bind_service=+eip lab9_1

echo -e "\nВозможности:"
getcap lab9_1
echo -e "\nxattr:"
xattr -l lab9_1
echo -e "\nС возможностями"
./lab9_1
echo -e "\nСнимаю возможности"
sudo setcap -r lab9_1
echo -e "\nВозможности:"
getcap lab9_1
echo -e "\nxattr:"
xattr -l lab9_1
echo -e "\nПосле снятия"
./lab9_1
