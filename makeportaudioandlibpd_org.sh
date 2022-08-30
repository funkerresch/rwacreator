cd "$(dirname "$0")"
if [ -f portaudio/lib/.libs/libportaudio.dylib ]
then echo "Found portaudio"
 else cd portaudio
 ./configure && make
 cd lib/.libs
 rm libportaudio.dylib
 cp libportaudio.2.dylib libportaudio.dylib
 rm libportaudio.2.dylib
 cd ..
 cd ..
 cd ..
 fi

if [ -f libpd/libs/libpd.dylib ]
then echo "Found libpd"
 else
 echo "compiling libpd"
 cd libpd
 make clean
 make EXTRA=true
 cd ..
 fi
 

