# Patient Interface and API Plan

## Goal
Add a patient-facing web flow where patients login by phone, submit an ESI form, view the nearest 3 hospitals with available matching or higher-level beds, and reserve a bed for 30 minutes.

## Data model
- `users`: staff accounts (coordinator, nurse, admin)
- `hospitals`: hospital information and optional coordinates
- `beds`: bed inventory with type, occupancy, and assigned patient
- `patients`: patient identities, phone login, and basic demographics
- `requests`: patient bed requests, including ESI and desired bed type
- `reservations`: temporary bed reservations for approved requests

## Suggested patient flow
1. Patient visits the portal and enters `phone_number`
2. If the phone exists in `patients`, authenticate; otherwise create a new patient record
3. Patient completes the bed request form:
   - name
   - age
   - symptoms / ESI level (1-4)
   - optional preferred hospital or location
4. Backend evaluates bed availability using the algorithm:
   - requested bed type or any higher-level bed type
   - unoccupied beds
   - active reservations count
5. Backend returns the nearest 3 hospitals with available beds
6. Patient selects one hospital/bed and reserves it for 30 minutes
7. Reservation is stored in `reservations` with `reserved_until`
8. The reservation expires automatically after 30 minutes if not fulfilled

## API endpoints

### Authentication
- `POST /api/auth/login`
  - body: `{ "phone_number": "..." }`
  - returns: `{ "patient_id", "phone_number", "name" }`

### Patient request
- `POST /api/patient/requests`
  - body: `{ "patient_id", "name", "age", "esi_level", "desired_bed_type", "preferred_hospital_id" }`
  - returns: `{ "request_id", "match_type", "suggested_hospitals": [ ... ] }`

- `GET /api/patient/requests/:request_id/availability`
  - returns the current top 3 hospitals and available bed counts

### Reservations
- `POST /api/patient/reservations`
  - body: `{ "request_id", "patient_id", "bed_id", "hospital_id" }`
  - returns reservation details and expiry timestamp

- `GET /api/patient/reservations/:reservation_id`
  - returns reservation status and timer

- `PATCH /api/patient/reservations/:reservation_id/cancel`
  - cancels the reservation

## Allocation rules
- ESI 1 -> ICU preferred
- ESI 2 -> Emergency bed preferred
- ESI 3 -> General bed preferred
- ESI 4 -> Specialty / any suitable bed
- If exact bed type is unavailable, allow a higher-level bed type
- Use the existing C++ allocation and nearest-hospital algorithms for matching and ranking

## Reservation expiration
- Create reservations with `reserved_until = NOW() + INTERVAL '30 minutes'`
- Periodically expire stale reservations using:
  - a cron / background job in Express
  - or a database trigger / scheduled job
- When a reservation expires, set `status = 'expired'` and free the `bed_id` if it was tentatively marked occupied

## Frontend pages
- `Login` by phone
- `Request bed` form
- `Choose hospital/bed` result list
- `Reservation confirmation` with timer
- `My reservations` summary

## Notes
- Keep C++ algorithm code for matching and nearest-hospital ranking
- Backend should still use PostgreSQL (`hospital_db`) and `pg` in Express
- Patient login by `phone_number` can be implemented directly from `patients` instead of `users`
