"""
GG 5.05 server debug script - registration and login test.
Usage: python test_register_login.py [host] [http_port] [chat_port]
"""

import socket
import struct
import sys
import urllib.request
import urllib.parse

# --- Config ---
HOST      = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
HTTP_PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 8080
CHAT_PORT = int(sys.argv[3]) if len(sys.argv) > 3 else 8074

UIN = 1041
PASSWORD  = "testpass"
GG_VERSION = 0x29  # GG 5.0


# --- Helpers ---

def gg_hash(password: str, seed: int) -> int:
    """GG32 login hash (same algorithm as in helpers.c)."""
    x = 0
    y = seed
    for c in password:
        x = (x & 0xffffff00) | ord(c)
        y ^= x
        y  = (y + x) & 0xFFFFFFFF
        x  = (x << 8) & 0xFFFFFFFF
        y ^= x
        x  = (x << 8) & 0xFFFFFFFF
        y  = (y - x) & 0xFFFFFFFF
        x  = (x << 8) & 0xFFFFFFFF
        y ^= x
        z  = y & 0x1f
        y  = ((y << z) | (y >> (32 - z))) & 0xFFFFFFFF
    return y


def recv_packet(s: socket.socket) -> tuple[int, bytes]:
    """Receive one GG packet, return (type, body)."""
    raw = s.recv(8)
    if len(raw) < 8:
        raise ConnectionError("Connection closed while reading header.")
    pkt_type, pkt_len = struct.unpack('<II', raw)
    body = b''
    while len(body) < pkt_len:
        chunk = s.recv(pkt_len - len(body))
        if not chunk:
            raise ConnectionError("Connection closed while reading body.")
        body += chunk
    return pkt_type, body


def send_packet(s: socket.socket, pkt_type: int, body: bytes) -> None:
    """Send one GG packet."""
    header = struct.pack('<II', pkt_type, len(body))
    s.sendall(header + body)


# --- Registration ---

def register(uin: int | None = None) -> tuple[int, str]:
    """
    Register a new account via HTTP (appmsg.asp / appmsg2.asp).
    Returns (uin, password) on success.
    """
    print(f"\n[REGISTER] Connecting to http://{HOST}:{HTTP_PORT}")

    params = urllib.parse.urlencode({
        "fmnumber":    UIN,
        "password": PASSWORD,
        "status":   "notavail",
    }).encode()

    url = f"http://{HOST}:{HTTP_PORT}/appsvc/fmregister2.asp"
    req = urllib.request.Request(url, data=params, method="POST")

    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            body = resp.read().decode(errors="replace")
    except Exception as e:
        print(f"[REGISTER] HTTP error: {e}")
        sys.exit(1)

    print(f"[REGISTER] Response:\n{body}")

    # Parse: expect a line like "uin:123456" or "UIN:123456"
    new_uin = None
    for line in body.splitlines():
        line = line.strip()
        if line.lower().startswith("uin:"):
            try:
                new_uin = int(line.split(":", 1)[1].strip())
            except ValueError:
                pass

    if new_uin is None:
        print("[REGISTER] Failed - could not parse UIN from response.")
        sys.exit(1)

    print(f"[REGISTER] OK - UIN: {new_uin}, password: {PASSWORD}")
    return new_uin, PASSWORD


# --- Login ---

def login(uin: int, password: str) -> None:
    """
    Log in via TCP chat server.
    Packet flow: GG_WELCOME (0x0001) -> GG_LOGIN (0x000C) -> GG_LOGIN_OK (0x0003) / GG_LOGIN_FAILED (0x0009)
    """
    print(f"\n[LOGIN] Connecting to {HOST}:{CHAT_PORT}")

    with socket.create_connection((HOST, CHAT_PORT), timeout=5) as s:

        # 1. Receive GG_WELCOME
        pkt_type, body = recv_packet(s)
        if pkt_type != 0x0001:
            print(f"[LOGIN] Expected GG_WELCOME (0x0001), got 0x{pkt_type:04X}")
            sys.exit(1)

        seed = struct.unpack('<I', body[:4])[0]
        print(f"[LOGIN] GG_WELCOME received, seed=0x{seed:08X}")

        # 2. Send GG_LOGIN (0x000C)
        hash_val = gg_hash(password, seed)
        status   = 0x0002  # GG_STATUS_AVAIL

        login_body = struct.pack('<IIII',
            uin,
            hash_val,
            status,
            GG_VERSION,
        )
        send_packet(s, 0x000C, login_body)
        print(f"[LOGIN] GG_LOGIN sent (uin={uin}, hash=0x{hash_val:08X})")

        # 3. Receive response
        pkt_type, body = recv_packet(s)

        if pkt_type == 0x0003:
            print("[LOGIN] GG_LOGIN_OK - SUCCESS")
        elif pkt_type == 0x0009:
            print("[LOGIN] GG_LOGIN_FAILED")
        else:
            print(f"[LOGIN] Unexpected packet: 0x{pkt_type:04X}, body={body.hex()}")


# --- Main ---

if __name__ == "__main__":
    uin, password = register()
    login(uin, password)