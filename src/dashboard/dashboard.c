/*
 * Serwer HTTP dla panelu Web Administer Interface
	(Interfejs Administracyjny sieci Web)
	
	Interfejs w HTTP sprawdzający statusy użytkowników
	i ich zalogowanie
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform.h"
#include "../server.h"
#include "../database/users.h"
#include "../chat/client.h"
#include "../chat/protocol.h"

#define BODY_SIZE  65536

static const char *status_str(uint32_t status) {
    switch (status & ~GG_STATUS_FRIENDS_MASK) {
        case GG_STATUS_AVAIL:           return "Dost&#281;pny";
        case GG_STATUS_AVAIL_DESCR:     return "Dost&#281;pny (opis)";
        case GG_STATUS_BUSY:            return "Zaraz wracam";
        case GG_STATUS_BUSY_DESCR:      return "Zaraz wracam (opis)";
        case GG_STATUS_INVISIBLE:       return "Niewidoczny";
        case GG_STATUS_NOT_AVAIL:       return "Niedost&#281;pny";
        case GG_STATUS_NOT_AVAIL_DESCR: return "Niedost&#281;pny (opis)";
        default:                        return "Nieznany";
    }
}

static void build_page(char *body, size_t body_size) {
    client_t **all     = client_get_all();
    User      *users   = db_get_all_users();
    int        count   = db_get_user_count();
    int        online  = 0;
    int        i;
    char       row[512];
    char       ver[16];

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (all[i] && all[i]->state == STATE_LOGIN_OK)
            online++;
    }

    snprintf(body, body_size,
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
        "<html>\n"
        "<head>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "<meta http-equiv=\"refresh\" content=\"10\">\n"
        "<title>OpenGaduServer - Panel WAI admina</title>\n"
        "<style type=\"text/css\">\n"
        "body{background:#f5f5f5;margin:16px;font-family:'Courier New',monospace;font-size:12px;color:#222;}\n"
        "h1{background:#222;color:#fff;text-align:center;padding:6px 0;font-size:13px;border-bottom:2px solid #888;margin:0;}\n"
        ".sub{background:#444;color:#ccc;font-size:10px;text-align:center;padding:3px 0;margin-bottom:10px;}\n"
        ".stats{display:flex;gap:8px;margin-bottom:10px;}\n"
        ".stat{flex:1;background:#fff;border:1px solid #aaa;padding:8px;text-align:center;}\n"
        ".stat-num{font-size:20px;font-weight:bold;display:block;color:#222;}\n"
        ".stat-lbl{font-size:10px;color:#888;}\n"
        "table{width:100%%;border-collapse:collapse;}\n"
        "th{background:#333;color:#fff;padding:5px 8px;border:1px solid #555;text-align:left;font-size:10px;}\n"
        "td{padding:4px 8px;border:1px solid #ccc;font-size:11px;color:#222;}\n"
        "tr:nth-child(even) td{background:#ececec;}\n"
        "tr:nth-child(odd) td{background:#ffffff;}\n"
        ".on{color:#222;font-weight:bold;}\n"
        ".off{color:#888;}\n"
        ".footer{color:#555;font-size:10px;text-align:center;margin-top:10px;border-top:1px solid #aaa;padding-top:6px;}\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<h1>&#9658; OpenGaduServer &mdash; WEB ADMINISTER INTERFACE</h1>\n"
        "<div class=\"sub\">Od&#347;wie&#380;anie co 10 sekund &bull; Wersja OGS 0.0.2</div>\n"
        "<div class=\"stats\">\n"
        "  <div class=\"stat\"><span class=\"stat-num\">%d</span><span class=\"stat-lbl\">Zarejestrowane Numerki</span></div>\n"
        "  <div class=\"stat\"><span class=\"stat-num\">%d</span><span class=\"stat-lbl\">Online</span></div>\n"
        "  <div class=\"stat\"><span class=\"stat-num\" style=\"color:#888;\">%d</span><span class=\"stat-lbl\">Offline</span></div>\n"
        "</div>\n"
        "<table>\n"
        "<tr>"
        "<th>Numer GG</th>"
        "<th>Zalogowany?</th>"
        "<th>Status</th>"
        "<th>Opis statusu</th>"
        "<th>Wersja klienta</th>"
        "<th>Adres IP</th>"
        "<th>Tylko znajomi</th>"
        "</tr>\n",
        count, online, count - online
    );

    for (i = 0; i < count; i++) {
        client_t *c = client_find(users[i].uin);
        int is_online = (c && c->state == STATE_LOGIN_OK);

        if (is_online)
            snprintf(ver, sizeof(ver), "0x%08X", c->version);
        else
            snprintf(ver, sizeof(ver), "&mdash;");

        snprintf(row, sizeof(row),
            "<tr>"
            "<td>%u</td>"
            "<td class=\"%s\">%s %s</td>"
            "<td>%s</td>"
            "<td>%s</td>"
            "<td>%s</td>"
            "<td>%s</td>"
            "<td>%s</td>"
            "</tr>\n",
            users[i].uin,
            is_online ? "on"      : "off",
            is_online ? "&#9679;" : "&#9675;",
            is_online ? "TAK"     : "NIE",
            is_online ? status_str(c->status) : "&mdash;",
            (is_online && c->status_descr && c->status_descr[0]) ? c->status_descr : "&mdash;",
            ver,
            is_online ? inet_ntoa(c->addr.sin_addr) : "&mdash;",
            (is_online && c->status_private) ? "TAK" : "NIE"
        );
        strncat(body, row, body_size - strlen(body) - 1);
    }

    strncat(body,
        "</table>\n"
        "<div class=\"footer\">"
        "Copyright &copy; 2026 Jakkret &bull; OpenGaduServer Web Administer Interface"
        "</div>\n"
        "</body>\n"
        "</html>\n",
        body_size - strlen(body) - 1
    );
}

static void handle_admin_client(int sock) {
    char  req[512] = {0};
    char *body;

    recv(sock, req, sizeof(req) - 1, 0);

    body = (char *)malloc(BODY_SIZE);
    if (!body) {
        close(sock);
        return;
    }

    memset(body, 0, BODY_SIZE);
    build_page(body, BODY_SIZE);

    char header[128];
    snprintf(header, sizeof(header),
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "\r\n"
    );
    send(sock, header, strlen(header), 0);
    send(sock, body, strlen(body), 0);

    free(body);
    close(sock);
}

void* http_admin_start(void *arg) {
    int srv, client;
    struct sockaddr_in addr, caddr;
    socklen_t clen = sizeof(caddr);
    int opt = 1;

    srv = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(sADMIN.IPaddr);
    addr.sin_port        = htons(HTTP_ADMIN_PORT);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERR("ADMIN: bind failed on port %d", HTTP_ADMIN_PORT);
        return NULL;
    }

    listen(srv, 5);

    while (1) {
        client = accept(srv, (struct sockaddr*)&caddr, &clen);
        if (client < 0) continue;
        handle_admin_client(client);
    }

    close(srv);
    return NULL;
}