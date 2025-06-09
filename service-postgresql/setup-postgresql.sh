#!/bin/bash

sudo cp -arvf postgresql/*.conf /etc/postgresql/16/main/
sudo chown postgres:postgres /etc/postgresql/16/main/*.conf

sudo -u postgres dropdb --if-exists spectrum
sudo -u postgres createdb spectrum
sudo -u postgres psql -d spectrum -f postgresql/schema.sql

sudo systemctl restart postgresql