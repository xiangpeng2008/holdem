cd $( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )

mkdir -p log
currentTime="`date +%Y%m%d_%H%M%S`"
kill $(ps aux | grep 'server.q -p 7779' | awk '{print $2}')
kill $(ps aux | grep '7778 holdem' | awk '{print $2}')
nohup ./server.cpp.o 20001 &> log/cpp_$currentTime.txt &
QHOME=~/q32 ~/q32/m32/q server.q -p 7779 &> log/server_$currentTime.txt
sleep 3
source ~/venv37/bin/activate
nohup gunicorn --bind 0.0.0.0:7778 holdem:server &> log/dash_$currentTime.txt &
#python holdem.py
