#!/bin/bash

set -e

DB_NAME="hospital_db"
SQL_COMMAND="psql"
CREATE_COMMAND="createdb"

if ! command -v psql >/dev/null 2>&1; then
  echo "psql command not found. Please install PostgreSQL client tools."
  exit 1
fi

if ! pg_isready -h localhost -p 5432 >/dev/null 2>&1; then
  echo "PostgreSQL is not running. Please start PostgreSQL."
  exit 1
fi

# Use sudo postgres if current user cannot access Postgres
if ! psql -c '\q' >/dev/null 2>&1; then
  if command -v sudo >/dev/null 2>&1; then
    SQL_COMMAND="sudo -u postgres psql"
    CREATE_COMMAND="sudo -u postgres createdb"
  else
    echo "Unable to connect to PostgreSQL as current user, and sudo is unavailable."
    exit 1
  fi
fi

echo "Ensuring database '$DB_NAME' exists..."
$CREATE_COMMAND "$DB_NAME" 2>/dev/null || true

echo "Applying schema..."
$SQL_COMMAND -d "$DB_NAME" -f hospital_db_schema.sql

echo "Clearing existing data..."
$SQL_COMMAND -d "$DB_NAME" -c "TRUNCATE reservations, requests, beds, patients, users, hospitals RESTART IDENTITY CASCADE;"

echo "Reloading seed data..."
$SQL_COMMAND -d "$DB_NAME" -f seed_data.sql

echo "Dummy data reset complete."
