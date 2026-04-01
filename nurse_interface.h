#ifndef NURSE_INTERFACE_H
#define NURSE_INTERFACE_H

#include "database.h"
#include "basic_structs.h"
#include <vector>

void nurseInterface(Database& db, std::vector<Bed>& beds, int hospital_id);

#endif