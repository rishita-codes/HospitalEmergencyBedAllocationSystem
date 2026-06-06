const express = require('express');
const pool = require('../config/db');
const { authenticateStaff, requireRole, authorizeBedHospital } = require('../middleware/auth');

const router = express.Router();

router.put('/beds/:bedId/assign', authenticateStaff, requireRole('coordinator'), authorizeBedHospital, async (req, res) => {
  try {
    const { bedId } = req.params;
    const { patient_id } = req.body;

    if (!patient_id) {
      return res.status(400).json({ error: 'patient_id is required' });
    }

    const result = await pool.query(
      `UPDATE beds
       SET is_occupied = true, assigned_patient_id = $1
       WHERE bed_id = $2
         AND is_occupied = false
         AND NOT EXISTS (
           SELECT 1 FROM reservations
           WHERE bed_id = $2 AND status = 'active' AND reserved_until > NOW()
         )
       RETURNING bed_id, hospital_id`,
      [patient_id, bedId]
    );

    if (result.rowCount === 0) {
      return res.status(404).json({ error: 'Bed not found or already occupied' });
    }

    await pool.query(
      `UPDATE requests SET status = 'matched'
       WHERE request_id = (
         SELECT request_id FROM requests
         WHERE patient_id = $1 AND preferred_hospital_id = $2 AND status = 'pending'
         ORDER BY created_at ASC
         LIMIT 1
       )`,
      [patient_id, req.staff.hospitalId]
    );

    res.json({ bed_id: bedId, patient_id });
  } catch (error) {
    console.error('Assign bed error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

router.put('/beds/:bedId/free', authenticateStaff, requireRole('coordinator', 'nurse'), authorizeBedHospital, async (req, res) => {
  try {
    const { bedId } = req.params;

    const bedResult = await pool.query(
      `SELECT assigned_patient_id, hospital_id
       FROM beds
       WHERE bed_id = $1`,
      [bedId]
    );

    if (bedResult.rows.length === 0) {
      return res.status(404).json({ error: 'Bed not found' });
    }

    const assignedPatientId = bedResult.rows[0].assigned_patient_id;
    const hospitalId = bedResult.rows[0].hospital_id;

    const result = await pool.query(
      `UPDATE beds
       SET is_occupied = false, assigned_patient_id = NULL
       WHERE bed_id = $1
       RETURNING bed_id`,
      [bedId]
    );

    if (result.rowCount === 0) {
      return res.status(404).json({ error: 'Bed not found' });
    }

    if (assignedPatientId) {
      await pool.query(
        `UPDATE requests
         SET status = 'cancelled'
         WHERE request_id = (
           SELECT request_id
           FROM requests
           WHERE patient_id = $1
             AND preferred_hospital_id = $2
             AND status IN ('pending', 'matched')
           ORDER BY created_at DESC
           LIMIT 1
         )`,
        [assignedPatientId, hospitalId]
      );
    }

    res.json({ bed_id: bedId, freed: true });
  } catch (error) {
    console.error('Free bed error:', error);
    res.status(500).json({ error: 'Internal server error' });
  }
});

module.exports = router;
