import csv
import time
import subprocess
from datetime import datetime

def get_pi5_total_power():
    """Lee y calcula el consumo total sumando todos los raíles del PMIC."""
    try:
        res = subprocess.check_output(['vcgencmd', 'pmic_read_adc'], stderr=subprocess.DEVNULL).decode('utf-8')
        
        # Diccionario para agrupar voltajes e intensidades por prefijo
        rails = {}
        
        for line in res.strip().split('\n'):
            if '=' not in line:
                continue
                
            # Separar el nombre (ej: "VDD_CORE_A current(7)") y el valor ("1.86637000A")
            parts = line.split('=')
            name_part = parts[0].strip().split(' ')[0] # Nos quedamos con "VDD_CORE_A"
            val_str = parts[1].strip()
            
            # Limpiar el valor numérico
            val = float(val_str.replace('V', '').replace('A', ''))
            
            # Extraer el prefijo (ej: VDD_CORE) y el tipo (V o A)
            prefijo = name_part[:-2]
            tipo = name_part[-1]
            
            if prefijo not in rails:
                rails[prefijo] = {'V': 0.0, 'A': 0.0}
            
            rails[prefijo][tipo] = val

        # Calcular el total sumando (V * A) de cada raíl
        total_power_w = 0.0
        core_power_w = 0.0
        
        for prefix, values in rails.items():
            # Si un raíl tiene tanto voltaje como intensidad, calculamos su potencia
            if 'V' in values and 'A' in values:
                power = values['V'] * values['A']
                total_power_w += power
                
                # Guardar el del Core aparte por si quieres graficar solo la CPU
                if prefix == 'VDD_CORE':
                    core_power_w = power

        return core_power_w, total_power_w
        
    except Exception as e:
        print(f"Error leyendo sensores: {e}")
        return 0.0, 0.0

def main():
    filename = "energia_total_old.csv"
    
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Timestamp", "Core_Power_W", "Total_Board_Power_W"])
        
        print("Iniciando monitorización de energía Total (PMIC Pi 5)...")
        print(f"Guardando en {filename}")
        print("-" * 50)
        print(f"{'Hora':<12} | {'Core Power (W)':<15} | {'Total Power (W)'}")
        print("-" * 50)
        
        try:
            while True:
                now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                core_p, total_p = get_pi5_total_power()
                
                if total_p > 0:
                    writer.writerow([now, round(core_p, 4), round(total_p, 4)])
                    file.flush()
                    print(f"{now[11:]:<12} | {core_p:<15.4f} | {total_p:.4f} W")
                
                time.sleep(1)
                
        except KeyboardInterrupt:
            print("\nMonitorización detenida.")

if __name__ == "__main__":
    main()