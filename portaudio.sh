cd "$(dirname "$0")"
if [ -f portaudio/lib/.libs/libportaudio.a ] 
then echo "yes"
 else cd portaudio 
 ./configure && make
 fi
