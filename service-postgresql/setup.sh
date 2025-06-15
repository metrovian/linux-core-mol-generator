#!/bin/bash

sudo apt install postgresql
sudo apt install postgresql-server-dev-16
git clone https://github.com/pgvector/pgvector.git
cd pgvector
make
sudo make install
cd ..
sudo rm -rfv pgvector

./setup-postgresql.sh