#!/bin/sh
# nagusia exekuzioaren pid-a lortu eta SIGINT seinalea bidali (Keyboard Interrupt)
# Exekuzioa bukatzeko, logbzb fitxategian lortutako nodoen informazioa idatziz eta 'q' eragiketa pantailaratzen mapa modu egokian ixteko (azken honek test.svg irudia sortuko du 'q' irakurtzean uneko mapa irudi bihurtuz)

#text=$(tail -1 "log1001")
#text=$(echo $text | cut -d ' ' -f 1)
#echo $text

#if [ "$(echo "$text" | cut -c 1-7)" = "BUKATU" ];
#if [ $? -eq 0 ];
#then
#    echo BA
#else
#    echo EZ
#fi

pidn=$(ps -aux | grep "nagusia" | grep -v "grep" | awk '{print $2}')
#echo $pidn
kill -2 $pidn

