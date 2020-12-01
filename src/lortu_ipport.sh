#!/bin/sh
# Run: sh lortu_ipport.sh
# When connected to monero daemon
# Pareen zerrenda lortu eta IP:Port formatuarekin idatzi

# RPC portuari eskatu pareen zerrenda
mkdir -p log
curl http://127.0.0.1:18081/get_peer_list -H 'Content-Type: application/json' > log/get_peer_list 2>log/log

host=$(cat log/get_peer_list | grep "host" | sed "s/::ffff://g" | cut -d ":" -f 2 | tr -d '"' | tr -d ',' )
port=$(cat log/get_peer_list | grep "port" | grep -v "rpc" | cut -d ":" -f 2 | tr -d ',')
echo "$host" | sed '/^\s*$/d' | sed  's/\r$//g' > log/ips
echo "$port" | sed '/^\s*$/d' | sed  's/\r$//g' > log/ports
iter=0

while read -r lerroa;
do
     iter=$((iter+1))
     porti=$(cat log/ports | sed -n "$iter p")

     if [ -z "$lerroa" ]
     then
         break
     fi
     if [ -z "$porti" ]
     then
         false
     else
         printf "%s:%s\n" $lerroa $porti
         #printf "\t\t:%s"  $porti
         #echo ""
         porti=""
     fi
done < log/ips
# Ongi egin duen egiaztatu
total=$(echo $iter)
atam=$(wc -l log/ips | cut -d " " -f 1)
btam=$(wc -l log/ports | cut -d " " -f 1)

if [ $total -eq $btam ]
then
    echo "Programa ongi bukatu da ("$total")" >&2
    exit 0
else
    echo i:$total a:$atam b:$btam  >&2
    exit 1
fi



#info=$(curl http://127.0.0.1:18081/get_peer_list -H 'Content-Type: application/json')
# curl http://127.0.0.1:18081/json_rpc -d '{"jsonrpc":"2.0","id":"0","method":"sync_info"}' -H 'Content-Type: application/json' > deitu_sync_info

