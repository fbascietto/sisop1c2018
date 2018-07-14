#!/bin/bash
# Deploy Destino Rusia

cd /home/utnso/

git clone https://github.com/sisoputnfrba/so-commons-library.git
git clone https://github.com/sisoputnfrba/parsi
git clone https://github.com/sisoputnfrba/Pruebas-ESI

cd /home/utnso/so-commons-library/
sudo make install

cd /home/utnso/parsi/src
sudo make install

cd /home/utnso/tp-2018-1c-Destino-Rusia/Biblioteca/makefiles
sudo make clean
sudo make

sudo cp libBiblioteca.so /usr/lib

cd /home/utnso/tp-2018-1c-Destino-Rusia/Coordinador/makefiles
sudo make clean
sudo make
cd /home/utnso/tp-2018-1c-Destino-Rusia/Instancia/makefiles
sudo make clean
sudo make
cd /home/utnso/tp-2018-1c-Destino-Rusia/Planificador/makefiles
sudo make clean
sudo make
cd /home/utnso/tp-2018-1c-Destino-Rusia/ESI/makefiles
sudo make clean
sudo make

cd /home/utnso/tp-2018-1c-Destino-Rusia/Instancia/

cp -rp makefiles instancia1
chmod 777 instancia1
cp -rp makefiles instancia2
chmod 777 instancia2
cp -rp makefiles instancia3 
chmod 777 instancia3
