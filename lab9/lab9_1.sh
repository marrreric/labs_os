#!/bin/bash
gcc lab9_1.c -o lab9_1 || exit 1
echo "Без возможностей"
./lab9_1
echo -e "\nУстанавливаю CAP_NET_BIND_SERVICE..."
sudo setcap cap_net_bind_service=+eip lab9_1
echo -e "\nС возможностями"
./lab9_1

