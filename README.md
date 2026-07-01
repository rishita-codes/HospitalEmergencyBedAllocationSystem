# Hospital Emergency Bed Allocation System

A full-stack web application for hospital emergency bed allocation with patient portal and staff management system.

## Architecture

- **Frontend**: React.js with routing
- **Backend**: Node.js/Express with PostgreSQL
- **Algorithms**: C++ implementation (greedy, optimal allocation, Dijkstra for routing)
- **Database**: PostgreSQL with 6 main tables

## Project Structure

```
hospital-ebas/
├── client/          # React frontend
├── server/          # Express backend
├── cpp/            # C++ algorithms
├── hospital_db_schema.sql
└── README.md
```

## Database Schema

The system uses 6 tables:
- `users` - Staff accounts (coordinator, nurse, admin)
- `hospitals` - Hospital information
- `beds` - Bed inventory and status
- `patients` - Patient identities (phone-based login)
- `requests` - Patient bed requests
- `reservations` - Temporary bed reservations

## Setup Instructions

### 1. Database Setup

```bash
# Create PostgreSQL database
createdb hospital_db

# Run schema
psql -d hospital_db -f hospital_db_schema.sql
```

### 2. Backend Setup

```bash
cd server
npm install
cp .env.example .env
# Adjust .env values if needed
npm start
```

The server runs on `http://localhost:3001`

### Resetting Dummy Data

If you want to restore the database to the seeded test state, run this from the repo root:

```bash
bash reset_db.sh
```

That will:
- ensure `hospital_db` exists
- apply the schema
- truncate all app tables
- reload `seed_data.sql`

It's ideal for repeated testing when you want a clean starting point.

### Staff login credentials
Use these dummy accounts for testing:

- Coordinator: `coord1` / `pass123`
- Nurse: `nurse1` / `nurse123`
- Admin: `admin1` / `admin123`

### Patient dummy login numbers
Example phone numbers seeded:

- `5551234567`
- `5559876543`
- `5552468101`
- `5557654321`

### 3. Frontend Setup

```bash
cd client
npm install
npm start
```

The React app runs on `http://localhost:3000`

### 4. C++ Algorithms (Optional)

```bash
cd cpp
# Compile the algorithms
g++ -std=c++17 *.cpp -I/usr/include/postgresql -lpq -o hospital_algorithm
```

## User Roles

### Patients
- Login with phone number
- Submit ESI assessment form
- View available hospitals
- Reserve beds for 30 minutes

### Coordinators
- View patient queue
- View bed status
- Run allocation algorithms
- Manage hospital resources

### Nurses
- Update bed occupancy
- Assign patients to beds
- Monitor bed status

## API Endpoints

### Patient APIs
- `POST /api/auth/patient-login` - Phone-based login
- `POST /api/patient/requests` - Create bed request
- `GET /api/patient/requests/:id/availability` - Get available hospitals
- `POST /api/patient/reservations` - Create bed reservation

### Staff APIs
- `POST /api/auth/staff-login` - Staff authentication
- `GET /api/hospitals/:id/beds` - Get hospital beds
- `GET /api/hospitals/:id/patients/waiting` - Get waiting patients
- `POST /api/allocate/:algorithm` - Run allocation algorithm

## ESI Level Determination

The system automatically determines ESI levels based on symptoms:
- **ESI 1**: Critical (chest pain, severe bleeding, unconsciousness)
- **ESI 2**: Urgent (moderate pain, high fever, dehydration)
- **ESI 3**: Less urgent (minor injuries, stable conditions)

## Allocation Algorithms

- **Greedy**: Fast allocation prioritizing highest ESI patients
- **Optimal**: Hungarian algorithm for maximum efficiency
- **Dijkstra**: Shortest path for hospital routing

## Reservation System

- Patients can reserve beds for 30 minutes
- Automatic expiration prevents holding
- Real-time availability checking
- Prevents double-booking

## Development

### Adding New Features

1. Update database schema if needed
2. Add API endpoints in `server/server.js`
3. Create React components in `client/src/components/`
4. Update routing in `client/src/App.js`

### Testing

```bash
# Backend tests
cd server
npm test

# Frontend tests
cd client
npm test
```

## Deployment

### Production Build

```bash
# Build React app
cd client
npm run build

# Start production server
cd server
NODE_ENV=production npm start
```

### Docker (Optional)

```dockerfile
# Dockerfile for the application
FROM node:16-alpine
WORKDIR /app
COPY package*.json ./
RUN npm install
COPY . .
EXPOSE 3001
CMD ["npm", "start"]
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes
4. Test thoroughly
5. Submit a pull request



## Environment & Secrets

- Keep real credentials out of the repository. Use a local `.env` file for development and add it to `.gitignore` (this repo already ignores `.env`).
- `server/.env.example` contains placeholders. Copy it to `server/.env` and fill real values only on your machine.
- Prefer a single `DATABASE_URL` environment variable for production deployments. Example:

```
DATABASE_URL=postgresql://postgres:your_password_here@localhost:5432/hospital_db
```
