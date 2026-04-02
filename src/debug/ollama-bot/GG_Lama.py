"""
GG 5.05 bot - łączy się z serwerem GG i odpowiada przez Ollama API.
Użycie: python GG_lama.py
wymagane biblioteki: requests
"""

import socket
import struct
import time
import requests

# konfiguracja
GG_HOST     = "192.168.137.1"
GG_PORT     = 8074

BOT_UIN     = 1041          # GG UIN
BOT_PASS    = "bott1"  # Password
GG_VERSION  = 0x29       # GG 5.0

OLLAMA_URL   = "http://localhost:11434/api/chat"
OLLAMA_MODEL = "Qwen2.5-Coder:7B-Instruct-Q5_K_M"  # Available model on your PC

# "jak masz się zachowywać?"
SYSTEM_PROMPT = """Jesteś asystentem o imieniu Lamus dostępnym przez komunikator Gadu-Gadu.
Odpowiadaj krótko i po polsku. Pamiętaj że rozmawiasz przez stary komunikator z 2004 roku. Bądź jak z lat 2000-2006 gdzie jeszcze telefony były na przyciski.

Natomiast kiedy '/help' to podajesz komendy jakie potrafisz obsłużyć. przypominaj co 5 minut żeby kończyć konwersacje komendą '/bye', ale jedynie przypominaj tym numerkom które rozmawiają z tobą. 

Kiedy ktoś napisze ci '/test err' to musisz wysłać '!! Błąd bota GaduBot - spróbuj napisać później' dopóki nie będzie wysłane '/halt err'. 

Kiedy ktoś ci napiszę '/test' to odpisz: '<jupi> Test udany! <spoko> (GG_Lama.py dla OGS-debug)', jeżeli wszystko powiodło się poprawnie.

kiedy użytkownik wita się z tobą, witaj się taką wiadomością: '<słonko> Witaj! Jestem Lamus działającym na API Ollamy (testowy skrypt dla lokalnego projektu OpenGaduServer). Mam nadzieję, że będzie miło nam się pisać! <pisze>' i w nowej linii pytasz użytkownika czy w czymś potrzebuje pomocy - tak jak zawsze robisz.

kiedy użytkownik napiszę ci: '/papuga' to masz powtarzać wszystko co użytkownik piszę, dopóki nie będzie wysłane: '/halt papuga'

Jeżeli użytkownik napiszę ci '/bye', żegnaj się taką wiadomością: '<papa> Mam nadzieję że zobaczymy się niedługo! <zegar>'."""


#   PROTOKÓŁ GG - funkcje pomocnicze 

def gg_hash(password: str, seed: int) -> int:
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


def recv_packet(s: socket.socket):
    """Odbierz jeden pakiet GG. Zwraca (type, body) albo None jeśli rozłączono."""
    # najpierw czytamy 8 bajtów nagłówka
    raw = b""
    while len(raw) < 8:
        chunk = s.recv(8 - len(raw))
        if not chunk:
            return None
        raw += chunk

    # rozpakowujemy dwa uint32 little-endian: typ i długość
    # "<" = little-endian, "II" = dwa unsigned int 32-bit
    pkt_type, pkt_len = struct.unpack("<II", raw)

    # teraz czytamy tyle bajtów ile mówi nagłówek
    body = b""
    while len(body) < pkt_len:
        chunk = s.recv(pkt_len - len(body))
        if not chunk:
            return None
        body += chunk

    return pkt_type, body


def send_packet(s: socket.socket, pkt_type: int, body: bytes):
    """Wyślij jeden pakiet GG."""
    # pakujemy nagłówek: typ + długość body, potem samo body
    header = struct.pack("<II", pkt_type, len(body))
    s.sendall(header + body)


def send_message(s: socket.socket, recipient: int, text: str):
    """Wyślij wiadomość GG_SEND_MSG (0x000B) do odbiorcy."""
    msg_bytes = text.encode("cp1250") + b"\x00"  # GG używa cp1250 + null terminator
    body = struct.pack("<III", recipient, int(time.time()), 0x0004) + msg_bytes
    send_packet(s, 0x000B, body)



#   OLLAMA - zapytanie do modelu

# historia rozmów per UIN - słownik: { uin: [ {role, content}, ... ] }
# dzięki temu bot pamięta kontekst każdej rozmowy osobno
conversations = {}

def ask_ollama(sender_uin: int, user_message: str) -> str:
    """Wyślij wiadomość do Ollamy i zwróć odpowiedź."""

    if sender_uin not in conversations:
        conversations[sender_uin] = []

    history = conversations[sender_uin]

    history.append({
        "role": "user",
        "content": user_message
    })

    payload = {
        "model": OLLAMA_MODEL,
        "stream": False,        # False = czekaj na całą odpowiedź
        "messages": [
            {"role": "system", "content": SYSTEM_PROMPT}
        ] + history             # system prompt + cała historia
    }

    try:
        # POST do lokalnego API Ollamy
        resp = requests.post(OLLAMA_URL, json=payload, timeout=60)
        resp.raise_for_status()  # rzuć błąd jeśli status != 200

        data = resp.json()
        answer = data["message"]["content"]

        # dodaj odpowiedź bota do historii
        history.append({
            "role": "assistant",
            "content": answer
        })

        # ogranicz historię do 20 wiadomości żeby nie puchła w nieskończoność
        if len(history) > 20:
            conversations[sender_uin] = history[-20:]

        return answer

    # używaj emotki z GG
    except requests.exceptions.ConnectionError:
        return "!! błąd: nie mogę połączyć się z Ollamą. Czy API działa? <czyta gazete>"
    except Exception as e:
        return f"!! błąd: {e}. Zgłoś ten błąd adminowi hostującego Lamę <telefon>"


def main():
    print(f"[BOT] Łączę się z serwerem GG {GG_HOST}:{GG_PORT}...")

    s = socket.create_connection((GG_HOST, GG_PORT), timeout=10)

    # connect like a client - get GG_WELCOME
    result = recv_packet(s)
    if not result or result[0] != 0x0001:
        print("[BOT] Nie otrzymałem GG_WELCOME!")
        return

    pkt_type, body = result
    seed = struct.unpack("<I", body[:4])[0]
    print(f"[BOT] Otrzymałem GG_WELCOME")

    # sending GG_LOGIN of v5 protocol
    hash_val = gg_hash(BOT_PASS, seed)

    login_body = struct.pack("<IIIIIh",
        BOT_UIN,
        hash_val,
        0x0002,      # GG_STATUS_AVAIL
        GG_VERSION,
        0,           # local_ip = 0.0.0.0
        0,           # local_port = 0
    )
    send_packet(s, 0x000C, login_body)
    print(f"[BOT] GG_LOGIN wysłany (UIN={BOT_UIN})")

    # --- KROK 3: odbierz odpowiedź logowania ---
    result = recv_packet(s)
    if not result:
        print("[BOT] Rozłączono podczas logowania!")
        return

    pkt_type, body = result
    if pkt_type == 0x0003:
        print("[BOT] Zalogowano pomyślnie!")
    elif pkt_type == 0x0009:
        print("[BOT] Logowanie nieudane (zły UIN lub hasło)!")
        return
    else:
        print(f"[BOT] Nieoczekiwany pakiet: 0x{pkt_type:04X}")
        return

    # --- KROK 4: wyślij pustą listę kontaktów (GG_LIST_EMPTY 0x0012) ---
    # bot nie ma kontaktów, ale musi poinformować serwer
    send_packet(s, 0x0012, b"")

    print("[BOT] Czekam na wiadomości...\n")
    s.settimeout(None)

    # --- KROK 5: główna pętla - nasłuchuj pakietów ---
    while True:
        result = recv_packet(s)
        if not result:
            print("[BOT] Połączenie zerwane.")
            break

        pkt_type, body = result

        # GG_RECV_MSG (0x000A) - ktoś wysłał wiadomość do bota
        if pkt_type == 0x000A:
            if len(body) < 16:
                continue

            # rozpakuj nagłówek wiadomości
            # sender=uint32, seq=uint32, time=uint32, class=uint32, potem tekst
            sender, seq, msg_time, msg_class = struct.unpack("<IIII", body[:16])
            
            # tekst to reszta body do pierwszego null bajtu
            text_raw = body[16:]
            null_pos = text_raw.find(b"\x00")
            if null_pos == -1:
                continue
            text = text_raw[:null_pos].decode("cp1250", errors="replace")

            print(f"[UŻYTKOWNIK] Wiadomość od {sender}: {text}")

            # zapytaj Ollamę i odeślij odpowiedź
            print(f"[BOT] Pytam Ollamę...")
            answer = ask_ollama(sender, text)
            print(f"[BOT] Odpowiedź: {answer}")

            send_message(s, sender, answer)

        # GG_PING (0x0008) - serwer pyta czy żyjemy, odpowiadamy GG_PONG
        elif pkt_type == 0x0008:
            send_packet(s, 0x0008, b"")

        # GG_DISCONNECTING (0x000B) - serwer nas rozłącza
        elif pkt_type == 0x000B:
            print("[BOT] Serwer rozłączył bota.")
            break


if __name__ == "__main__":
    main()