##### IPK: Projekt 1, varianta 2 #####
##### Vypracoval: Alex Sporni #####
##### xsporn01@stud.fit.vutbr.cz #####
##### My APPID: b30488c9fb2d604c3ecc44b050c5d662 #####
from socket import *
import json
import sys

### Socket ###
serverName = "api.openweathermap.org"
serverPort = 80
try:
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName, serverPort))
    clientSocket.sendall(bytes("GET /data/2.5/weather?q={0}&appid={1}&units=metric HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n".format(str(sys.argv[2]).lower(),str(sys.argv[1])),"utf-8"))
    modifVstup = clientSocket.recv(1024)
    clientSocket.close()
except:
    print("An error occurred while connecting to the server", file=sys.stderr)
    exit(1)
### Parsing ###
better = modifVstup.decode()
position = better.find("{")
final = better[position:]
json_conv = json.loads(final)
errCode = json_conv["cod"]
if errCode != 200:
    errMsg = json_conv["message"]
    print("Error code is: {}".format(errCode)," meaning: {}".format(errMsg))
    exit(1)
mesto = json_conv["name"]
pocasie = json_conv["weather"][0]["description"]
teplota = json_conv["main"]["temp"]
vlhkost = json_conv["main"]["humidity"]
tlak = json_conv["main"]["pressure"]
rychlost_vetra = json_conv["wind"]["speed"]
rychlost_vetra_kmh = rychlost_vetra * 3.6

### Printing ###
print("{}".format(mesto))
print("{}".format(pocasie))
print("temp: {}\N{DEGREE SIGN}C".format(teplota))
print("humidity: {}%".format(vlhkost))
print("preassure: {} hPa".format(tlak))
print("wind-speed: {0:.2f}km/h".format(rychlost_vetra_kmh))
try:
    smer_vetra = json_conv["wind"]["deg"]
except:
    print("wind-deg: " + "Unavailible")
else:
    print("wind-deg: {}".format(smer_vetra))