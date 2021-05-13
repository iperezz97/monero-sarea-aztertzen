# monero-sarea-aztertzen

Proiektua exekutatzeko, kodea direktorioan kokatuta:
  - geoip karpeta kokatu edo get_maxmind.sh script-a exekutatu
  - ondorengo komandoak exekutatu:

     Konpilatu: 

        ``` gcc nagusia.c bzb.c eskatu1001.c konprob1003.c kokatu_mapan.c jaso2002.c -lpthread -o nagusia```
  
     Exekutatu: 
        
        ``` ./nagusia 212.83.175.67 18080 1 | python kokatu_mapan.py```
