import os
import time
import json
import csv
import threading
import queue
import paho.mqtt.client as mqtt
import signal
import pandas as pd

# Configuración
BROKER = "192.168.5.191"  # Cambiar por la IP del broker
PORT = 30511
HOSTNAME = os.uname()[1].split("-")[1]
TOPIC = "opentwins-realtime/twin/events/lightweigh/twin{}".format(HOSTNAME)

if not os.path.exists("validation/test2"):
    os.makedirs("validation/test2") 
else:
    None
CSV_FILE = "validation/test2/resultados_latencia-rbpi{}.csv".format(HOSTNAME)

# Cola para desacoplar la recepción de mensajes de la escritura en disco
lista_mensajes = []


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print(f"Conectado al broker. Suscribiéndose a '{TOPIC}'...")
        client.subscribe(TOPIC, qos=1)
    else:
        print(f"Fallo al conectar. Código: {reason_code}")

def on_message(client, userdata, msg):
    # 1. Capturar el timestamp de llegada lo antes posible para mayor precisión
    t_recepcion = time.time_ns()
    
    try:
        # 2. Decodificar el payload
        payload = json.loads(msg.payload.decode('utf-8'))
        payload["timestamp_recepcion"] = t_recepcion
        
        # 3. Enviar a la cola de escritura
        lista_mensajes.append(payload)
    except Exception as e:
        print(f"Error procesando mensaje: {e}")


# Configurar cliente MQTT
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)


client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)


print("Escuchando mensajes... Presiona Ctrl+C para finalizar.")
try:
    client.loop_forever()
except KeyboardInterrupt:
    print("\nDesconectando y guardando datos restantes...")
    client.disconnect()
    df = pd.DataFrame(lista_mensajes)
    df.to_csv(CSV_FILE, index=False)
    
    print(f"Datos guardados en {CSV_FILE}.")