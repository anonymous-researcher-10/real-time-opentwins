import zmq
import msgpack
import time
import pandas as pd
# --- Constantes que coinciden con tu event_types.h en C ---
EVENT_UPDATE = 0
EVENT_ERROR = 1

ERROR_SENSOR_FAILURE = 1 # Según tu error_type_t

def recibir_eventos():
    # 1. Configurar ZeroMQ
    context = zmq.Context()
    socket = context.socket(zmq.SUB)

    socket.setsockopt(zmq.RCVHWM, 10)
    
    socket.connect("tcp://192.168.48.206:5556")
    socket.setsockopt(zmq.SUBSCRIBE, b"") # Suscribirse a todos los mensajes
    
    mensajes = []
    
    sending_ts = 0
    
    try:
    
        while True:
            time.sleep(0.1) # Evitar CPU al 100% en espera

            # Recepcion de mensajes
            try:
                msg = socket.recv(zmq.NOBLOCK) # No bloquear, para poder salir del loop
                
                msg = msgpack.unpackb(msg, raw=False) # Desempaquetar el mensaje

                actual_ts = int(time.time() * 1000)
                sending_ts = msg[1]
                mensajes.append((sending_ts, actual_ts))
                
            except zmq.Again:
                continue # No hay mensaje, seguir esperando
            except Exception as e:
                print(f"Error: {e}")
    except KeyboardInterrupt:
        mensajes_df = pd.DataFrame(mensajes, columns=["Sending_Timestamp", "Receiving_Timestamp"])
        mensajes_df.to_csv("utils_testing/mensajes_recibidos2.csv", index=False)
        print("\n[PYTHON] Recepción finalizada.")
   
    # Limpieza
    socket.close()
    context.term()
    print("\n[PYTHON] Transmisión finalizada.")

if __name__ == "__main__":
    recibir_eventos()