#!/bin/bash

# Hospital EBAS Setup Script

echo "Setting up Hospital Emergency Bed Allocation System..."

# Check if PostgreSQL is running
if ! pg_isready -h localhost -p 5432 > /dev/null 2>&1; then
    echo "PostgreSQL is not running. Please start PostgreSQL service."
    exit 1
fi

# Create database
echo "Creating database..."
createdb hospital_db 2>/dev/null || echo "Database already exists"

# Run schema
echo "Setting up database schema..."
psql -d hospital_db -f hospital_db_schema.sql

# Load dummy seed data
if [ -f seed_data.sql ]; then
    echo "Loading dummy seed data..."
    psql -d hospital_db -f seed_data.sql
else
    echo "Warning: seed_data.sql not found, skipping dummy data load."
fi

echo "Database setup complete!"

# Install backend dependencies
echo "Installing backend dependencies..."
cd server
npm install

# Build C++ allocator binary
echo "Building C++ allocator..."
g++ -std=c++17 \
    cpp/allocator_cli.cpp cpp/database.cpp cpp/basic_structs.cpp cpp/priority_queue.cpp \
    cpp/greedy_allocator.cpp cpp/optimal_allocator.cpp cpp/bipartite_matching.cpp cpp/dijkstra.cpp \
    -I/usr/include/postgresql -lpq -O2 -o cpp/hospital_allocator 2>/dev/null || echo "C++ build failed or dependencies missing; please install libpq-dev and g++"

# Install frontend dependencies
echo "Installing frontend dependencies..."
cd ../client
npm install

cd ..

echo "Setup complete!"
echo ""
echo "To start the application:"
echo "1. Terminal 1: cd server && npm start"
echo "2. Terminal 2: cd client && npm start"
echo ""
echo "Access the application at:"
echo "- Frontend: http://localhost:3000"
echo "- Backend API: http://localhost:3001"