# OpenGaduServer
independent server compatible with Gadu-Gadu clients from 2000's for self-hosting use. The source code is written in C and relies on community documentation such as [libgadu](https://libgadu.github.io/protocol) (sources listed in `SOURCES` file). For now the project is working on Windows (Linux support is in the way) and as of now focuses on supporting Gadu Gadu 5.x clients (check Support List)


> [!IMPORTANT]
> This Project is not affiliated with, endorsed by or associated with Gadu Gadu (or GG). This project is entirely non-commercial and does not get any revenue off it.

------------------------------
### Client version and Protocol Support List
Currently, OpenGaduServer supports most basic functions of:
- [ ] Gadu Gadu 3.1: (optional as of now)
    - [ ] register and appsvc routing
    - [ ] resetting password
    - [ ] GG_WELCOME
    - [ ] GG_LOGIN (`struct GG_login3`)
    - [ ] Chat functionality

- [x] Gadu Gadu 5.0:
    - [x] register and appsvc routing
    - [ ] resetting password
    - [x] GG_WELCOME
    - [x] GG_LOGIN (`struct GG_login5`)
    - [ ] Chat functionality
    - [ ] fetch contact list
