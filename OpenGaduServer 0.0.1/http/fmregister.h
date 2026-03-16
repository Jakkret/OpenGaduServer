#ifndef FMREGISTER_H
#define FMREGISTER_H

// Handle GG3 registration request
// POST /fmregister.php
// Params: email, pwd, code, tokenid, tokenval
void handle_fmregister(int sock, char *query);

#endif // FMREGISTER_H