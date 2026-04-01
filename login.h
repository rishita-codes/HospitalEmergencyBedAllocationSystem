#ifndef LOGIN_H
#define LOGIN_H

#include "database.h"
#include "basic_structs.h"
#include <vector>

User handleLogin(Database& db);
void startSession(Database& db, User u, std::vector<Bed>& beds);

#endif