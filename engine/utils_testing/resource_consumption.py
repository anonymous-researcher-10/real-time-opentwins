import psutil
import csv
import time
from datetime import datetime
import os

def monitor_rt_to_csv(filename="monitorizacion_rt.csv"):
    # 1. Obtener la cantidad real de núcleos para crear las columnas dinámicamente
    num_cores = psutil.cpu_count()
    headers = ["Timestamp"] + [f"Core_{i}_%" for i in range(num_cores)] + [
        "RAM_%", "RAM_Used_GB", "RAM_Total_GB", "Download_MB_s", "Upload_MB_s"
    ]
    
    file_exists = os.path.isfile(filename)
    
    print(f"Iniciando monitorización.")
    print(f"Guardando datos en: {filename}")
    print("Presiona Ctrl+C para detener y guardar.\n")
    
    # 2. Abrir archivo en modo 'append' (añadir) para no sobreescribir si reinicias el script
    with open(filename, mode='a', newline='') as file:
        writer = csv.writer(file)
        
        # Escribir cabeceras solo si es un archivo nuevo
        if not file_exists:
            writer.writerow(headers)
            
        net_io_inicio = psutil.net_io_counters()
        
        try:
            while True:
                # Obtener la hora actual
                now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                
                # CPU Per-Core (vital para tus núcleos aislados)
                cpu_cores = psutil.cpu_percent(interval=1, percpu=True)
                
                # Memoria
                memoria = psutil.virtual_memory()
                mem_usada_gb = memoria.used / (1024 ** 3)
                mem_total_gb = memoria.total / (1024 ** 3)
                
                # Red
                net_io_fin = psutil.net_io_counters()
                mb_enviados = (net_io_fin.bytes_sent - net_io_inicio.bytes_sent) / (1024 ** 2)
                mb_recibidos = (net_io_fin.bytes_recv - net_io_inicio.bytes_recv) / (1024 ** 2)
                net_io_inicio = net_io_fin
                
                # 3. Preparar la fila para el CSV (Redondeando valores)
                row = [now] + cpu_cores + [
                    round(memoria.percent, 1), 
                    round(mem_usada_gb, 2), 
                    round(mem_total_gb, 2), 
                    round(mb_recibidos, 3), 
                    round(mb_enviados, 3)
                ]
                
                # 4. Escribir en el CSV y forzar volcado a disco
                writer.writerow(row)
                file.flush()  # <--- CRÍTICO: Asegura que la línea se escribe en disco inmediatamente
                
                # También imprimimos un pequeño resumen por terminal para que veas que funciona
                resumen_cpu = f"C0:{cpu_cores[0]:.0f}%" # Imprime el primer núcleo como referencia
                print(f"[{now}] {resumen_cpu} | RAM: {memoria.percent}% | Grabando fila...")
                
        except KeyboardInterrupt:
            print("\nMonitorización detenida. CSV guardado correctamente.")

if __name__ == "__main__":
    monitor_rt_to_csv()