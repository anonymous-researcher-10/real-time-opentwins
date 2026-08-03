import os
import time
import json
import paho.mqtt.client as mqtt

# Configuración del broker MQTT
BROKER = "192.168.5.191"  # Cambiar por la IP del broker
PORT = 30511

HOSTNAME = os.uname()[1].split("-")[1]
TOPIC = f"realtime/twin{HOSTNAME}"
print(HOSTNAME)

# Baterías de mensajes a enviar
BATERIAS = [10, 50, 100, 200, 500, 1000, 5000]

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("Conectado exitosamente al broker.")
    else:
        print(f"Fallo al conectar. Código: {reason_code}")

# Inicializar cliente MQTT (usando la API v2 recomendada para versiones recientes de paho-mqtt)
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect

client.connect(BROKER, PORT, 60)
client.loop_start()

contador = 0


##############################
# WARM-UP
##############################

for i in range(10):
    payload = {
        "id": "twin{}".format(HOSTNAME),
        "value" : contador,
        "sending_timestamp": time.time_ns()
    }
    
    # Publicar mensaje (QoS 1 garantiza la entrega al menos una vez)
    client.publish(TOPIC, json.dumps(payload), qos=1)
    time.sleep(0.5)


##############################
# PRUEBAS
##############################

for cantidad in BATERIAS:
    print(f"Iniciando envío de batería de {cantidad} mensajes...")
    
    for i in range(cantidad):
        
        payload = {
            "id": "twin{}".format(HOSTNAME),
            "value" : contador,
            "sending_timestamp": time.time_ns()
        }
        contador += 1
        
        # Publicar mensaje (QoS 1 garantiza la entrega al menos una vez)
        client.publish(TOPIC, json.dumps(payload), qos=1)
        
        time.sleep(0.1)
        
    print(f"Batería de {cantidad} enviada. Esperando 2 segundos...")
    time.sleep(30)

client.loop_stop()
client.disconnect()
print("Todos los datos han sido enviados.")