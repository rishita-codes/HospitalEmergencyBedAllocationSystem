#include "nurse_interface.h"
#include <iostream>
#include <vector>

void nurseInterface(Database& db, std::vector<Bed>& beds, int hospital_id) {
    while (true) {
        std::cout << "\n-----------------------------\n";
        std::cout << "NURSE MODE (Discharge)\n";
        std::cout << "1. Show occupied beds\n";
        std::cout << "2. Free a bed (discharge patient)\n";
        std::cout << "0. Logout\n";
        std::cout << "Enter choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                beds = db.getBeds(hospital_id);
                std::cout << "\nOccupied beds for your hospital:\n";
                for (Bed& b : beds) {
                    if (b.occupied()) {
                        std::cout << "  " << b.getBedId()
                                  << " -> " << b.getAssignedPatientId()
                                  << " (type=" << bedTypeToString(b.getType()) << ")\n";
                    }
                }
                break;
            }

            case 2: {
                std::cout << "Enter bed_id to free: ";
                std::string bid;
                std::cin >> bid;

                beds = db.getBeds(hospital_id);
                Bed* bed = nullptr;
                for (Bed& b : beds) {
                    if (b.getBedId() == bid && b.occupied()) {
                        bed = &b;
                        break;
                    }
                }

                if (!bed) {
                    std::cout << "Bed not found or not occupied.\n";
                    break;
                }

                std::string patient_id = bed->getAssignedPatientId();
                db.freeBed(bid);
                if (!patient_id.empty()) {
                    db.deletePatient(patient_id);
                }
                bed->freeBed();
                std::cout << "Bed " << bid << " is now free";
                if (!patient_id.empty()) {
                    std::cout << " and patient " << patient_id << " was discharged.";
                }
                std::cout << "\n";
                break;
            }

            case 0:
                std::cout << "Logging out...\n";
                return;

            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }
}