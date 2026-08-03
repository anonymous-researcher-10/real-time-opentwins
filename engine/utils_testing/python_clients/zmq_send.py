import zmq
import msgpack
import time
import pandas as pd
# --- Constantes que coinciden con tu event_types.h en C ---
EVENT_UPDATE = 0
EVENT_ERROR = 1

ERROR_SENSOR_FAILURE = 1 # Según tu error_type_t

pruebas = [10, 50, 100, 200, 500, 1000]


# 1. Configurar ZeroMQ
context = zmq.Context()
socket = context.socket(zmq.PUSH)


socket.setsockopt(zmq.RCVHWM, 10)

socket.connect("tcp://192.168.48.206:5555")

print("[PYTHON] Conectado a ZeroMQ en el puerto 5555.")
# ZMQ necesita unos milisegundos para establecer la conexión TCP por debajo
time.sleep(1) 

# ==========================================
# ENVIAR EVENTO DE UPDATE (10 variables)
# ==========================================

pruebas_totales = []

twin_id = 1

total_vars = 1
deviation = 0.5


for total_msg in pruebas:
    msgs_enviados = []
    for i in range(total_msg):
        timestamp = int(time.time() * 1000)


        # Estructura esperada por tu C: [TYPE, TWIN_ID, TIMESTAMP, TOTAL_VARS, id1, val1, id2, val2...]
        payload_update = [EVENT_UPDATE, timestamp, deviation, twin_id, total_vars]

        # print(f"\n[PYTHON] Generando UPDATE para Gemelo {twin_id} con {total_vars} variables...")
        for i in range(total_vars):
            var_id = i
            valor = float((i + 1) * 10.5) # Ej: 10.5, 21.0, 31.5...
            
            payload_update.append(var_id)
            payload_update.append(valor)
            # print(f"  -> Empaquetando Var ID: {var_id} | Valor: {valor}")

        # Serializamos a MessagePack (use_single_float=True ayuda a que C lo lea directamente como float f32)
        msg_update = msgpack.packb(payload_update, use_single_float=True)

        socket.send(msg_update)
        # print(f"[PYTHON] UPDATE enviado ({len(msg_update)} bytes).")
        msgs_enviados.append(timestamp)
        time.sleep(0.5) # Pausa pequeña entre mensajes
    pruebas_totales.append((total_msg, msgs_enviados))
    time.sleep(30)

# # ==========================================
# # ENVIAR EVENTO DE ERROR
# # ==========================================

# # Estructura esperada por tu C: [TYPE, TWIN_ID, TIMESTAMP, ERROR_CODE, "Mensaje"]
# payload_error = [
#     EVENT_ERROR, 
#     timestamp, 
#     deviation, 
#     twin_id,
#     ERROR_SENSOR_FAILURE, 
#     "Fallo critico de presion"
# ]

# msg_error = msgpack.packb(payload_error)
# socket.send(msg_error)
# print(f"\n[PYTHON] ERROR enviado ({len(msg_error)} bytes).")

# Limpieza
socket.close()
context.term()
print("\n[PYTHON] Transmisión finalizada.")


df_pruebas = pd.DataFrame(pruebas_totales, columns=["Total_Msg", "Timestamps"])
df_pruebas.to_csv("pruebas_zmq.csv", index=False)