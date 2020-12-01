# monero-sarea-aztertzen

lortu_ipport.sh:

Monero sareari konektatuta egonda, 6000 nodoren informazioa lortu; IP eta portua sarrera estandarrean idatzi
Exekutatu:
$ sh lortu_ipport > ipport_emaitza 2> errors


port_scan.c:
Monero nodo bati SYN TCP paketea bidali; ACK bidaltzea itxaron eta jaso bidaltzen duen gainerako informazioa 

Konpilatu:
$ gcc port_scan.c -lpthread -o port_scan
Exekutatu:
$ sudo ./port_scan <IPAddress> <Port>
Adibidez:
$ sudo ./port_scan 65.60.130.0 18080
  
