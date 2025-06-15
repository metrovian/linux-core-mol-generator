#!/bin/bash

sudo mkdir -pv /var/www/nginx/
sudo mkdir -pv /var/www/html/

sudo cp -arv nginx/* /etc/nginx/
sudo cp -arv html/* /var/www/html/

sudo systemctl reload nginx

