import bluetooth
import subprocess
import time

time.sleep(15)

subprocess.call("(cd /var/www/html)", shell=True)

bd_addr = "98:D3:32:30:C3:13"
port = 1
sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)

sock.connect((bd_addr, port))

data=""
while 1:
    try:
        data = sock.recv(1024)
        #print data
        if data == 'o':
            proc = subprocess.Popen("php /var/www/html/authentic_notification.php", shell=True, stdout=subprocess.PIPE)
            script_response = proc.stdout.read()
        elif data == 'x':
            proc = subprocess.Popen("php /var/www/html/deauthentic_notification.php", shell=True, stdout=subprocess.PIPE)
            script_response = proc.stdout.read()
    except:
        sock.close()
        break

