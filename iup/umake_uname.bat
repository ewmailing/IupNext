#This builds all the libraries for 1 uname

cd src
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srccd
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srccontrols
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srcgl
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srcpplot
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srcim
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srcledc
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srcview
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

#cd srclua3
#./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
#cd ..

cd srclua5
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..

cd srcconsole
./umake_uname.bat $1 $2 $3 $4 $5 $6 $7 $8
cd ..
