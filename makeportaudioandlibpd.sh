cd "$(dirname "$0")"
 if [ -f portaudio/lib/.libs/libportaudio.dylib ]
 then echo "Found portaudio"
 else
 cd portaudio
 make clean
 ./configure && make
 cd ..
 fi

if [ -f libpd/libs/libpd.dylib ]
then echo "Found libpd"
 else
 echo "compiling libpd"
 cd libpd
 make clean
 make EXTRA=true FAT_LIB=true
 cd ..
 fi
 

