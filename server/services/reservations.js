const pool = require('../config/db');

async function expireReservations() {
  await pool.query(`
    UPDATE reservations
    SET status = 'expired'
    WHERE status = 'active' AND reserved_until < NOW()
  `);
}

module.exports = { expireReservations };
