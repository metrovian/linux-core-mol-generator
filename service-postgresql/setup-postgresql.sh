#!/bin/bash

sudo cp -arvf postgresql/* /etc/postgresql/16/main/
sudo chown postgres:postgres /etc/postgresql/16/main/*

sudo systemctl restart postgresql