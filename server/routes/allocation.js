const express = require('express');
const pool = require('../config/db');
const { authenticateStaff, authorizeHospital } = require('../middleware/auth');
const { runAllocator } = require('../services/cpp');
const { expireReservations } = require('../services/reservations');

const router = express.Router();

async function filterAssignments(assignments, hospital_id) {
  const filtered = [];

  for (const a of assignments) {
    const patientId = a.patient_id;
    const bedId = a.bed_id;

    const bedRes = await pool.query('SELECT hospital_id, is_occupied FROM beds WHERE bed_id = $1', [bedId]);
    if (bedRes.rows.length === 0) {
      console.warn('Allocator returned unknown bed:', bedId);
      continue;
    }
    if (String(bedRes.rows[0].hospital_id) !== String(hospital_id)) {
      console.warn('Skipping assignment for bed not in hospital:', bedId);
      continue;
    }
    if (bedRes.rows[0].is_occupied) {
      console.warn('Skipping occupied bed:', bedId);
      continue;
    }

    const reqRes = await pool.query(
      `SELECT request_id FROM requests WHERE patient_id = $1 AND preferred_hospital_id = $2 AND status = 'pending' ORDER BY created_at ASC LIMIT 1`,
      [patientId, hospital_id]
    );
    if (reqRes.rows.length === 0) {
      console.warn('Skipping assignment for patient without pending request at this hospital:', patientId);
      continue;
    }

    filtered.push({ patient_id: patientId, bed_id: bedId, request_id: reqRes.rows[0].request_id });
  }

  return filtered;
}

router.post('/allocate/:algorithm', authenticateStaff, authorizeHospital, async (req, res) => {
  try {
    await expireReservations();
    const { algorithm } = req.params;
    const { hospital_id } = req.body;

    if (!hospital_id) {
      return res.status(400).json({ error: 'hospital_id is required' });
    }

    const assignments = await runAllocator(algorithm, hospital_id);
    const filtered = await filterAssignments(assignments, hospital_id);

    const client = await pool.connect();
    try {
      await client.query('BEGIN');
      for (const a of filtered) {
        const { patient_id, bed_id, request_id } = a;

        await client.query(
          `UPDATE beds
           SET is_occupied = true, assigned_patient_id = $1
           WHERE bed_id = $2
             AND hospital_id = $3
             AND is_occupied = false
             AND NOT EXISTS (
               SELECT 1 FROM reservations
               WHERE bed_id = $2 AND status = 'active' AND reserved_until > NOW()
             )`,
          [patient_id, bed_id, hospital_id]
        );

        await client.query(`UPDATE requests SET status = 'matched' WHERE request_id = $1`, [request_id]);
      }
      await client.query('COMMIT');
      res.json({ algorithm, hospital_id, assignments: filtered });
    } catch (error) {
      await client.query('ROLLBACK');
      console.error('Persisting assignments failed', error);
      res.status(500).json({ error: 'Persist failed' });
    } finally {
      client.release();
    }
  } catch (error) {
    console.error('Allocation error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

router.post('/allocate/:algorithm/preview', authenticateStaff, authorizeHospital, async (req, res) => {
  try {
    await expireReservations();
    const { algorithm } = req.params;
    const { hospital_id } = req.body;

    if (!hospital_id) {
      return res.status(400).json({ error: 'hospital_id is required' });
    }

    const assignments = await runAllocator(algorithm, hospital_id);
    const filtered = await filterAssignments(assignments, hospital_id);
    res.json({ algorithm, hospital_id, assignments: filtered });
  } catch (error) {
    console.error('Allocation preview error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

module.exports = router;
