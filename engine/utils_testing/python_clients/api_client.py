import zmq
import msgpack
import sys

print("========================================")
print("🐍 EJECUTANDO BATERÍA DE TESTS RPC")
print("========================================\n")

context = zmq.Context()
socket = context.socket(zmq.REQ)
# Timeout de 2 segundos para que el test no se cuelgue si C falla
socket.setsockopt(zmq.RCVTIMEO, 2000) 
socket.connect("tcp://localhost:5556")

def rpc_call(comando_array):
    socket.send(msgpack.packb(comando_array))
    try:
        return msgpack.unpackb(socket.recv(), raw=False)
    except zmq.error.Again:
        print("❌ ERROR: El servidor C no respondió (Timeout).")
        sys.exit(1)

try:
    # --- TEST 1: Petición mal formada ---
    print("Test 1: Petición inválida (Enviando texto en vez de array)... ", end="")
    socket.send_string("hola")
    resp_err = msgpack.unpackb(socket.recv(), raw=False)
    assert resp_err.get("status") == "error", "Debería devolver un error"
    print("✅ PASS")

    # --- TEST 2: GET del Gemelo 6 ---
    print("Test 2: GET_TWIN (Opcode 1, ID 6)... ", end="")
    resp_get = rpc_call([1, 6])
    assert resp_get.get("id") == 6, "El ID devuelto no coincide"
    # Sabiendo que en C inyectamos un 85.5 en la var 0
    assert abs(resp_get["vars"][0] - 85.5) < 0.01, "El valor de la variable es incorrecto"
    print("✅ PASS")

    # --- TEST 3: DELETE del Gemelo 6 ---
    print("Test 3: DELETE_TWIN (Opcode 2, ID 6)... ", end="")
    resp_del = rpc_call([2, 6])
    assert resp_del.get("status") == "deleted", "El estado no cambió a deleted"
    assert resp_del.get("id") == 6, "El ID borrado no coincide"
    print("✅ PASS")

    # --- TEST 4: GET_ALL ---
    print("Test 4: GET_ALL (Opcode 0)... ", end="")
    resp_all = rpc_call([0])
    assert len(resp_all) == 10, "Debería devolver el array completo de 10 gemelos"
    # Verificamos que el Gemelo 6 esté en el array y tenga la variable a 85.5
    gemelo_6 = next((t for t in resp_all if t["id"] == 6), None)
    assert gemelo_6 is not None, "El gemelo 6 no está en la lista global"
    assert abs(gemelo_6["vars"][0] - 85.5) < 0.01, "El valor global no coincide"
    print("✅ PASS")

    print("\n🎉 TODOS LOS TESTS PASARON CORRECTAMENTE. EL PROTOCOLO ES ROBUSTO.")

except AssertionError as e:
    print(f"❌ FAIL: {e}")
finally:
    socket.setsockopt(zmq.LINGER, 0)
    socket.close()
    context.term()