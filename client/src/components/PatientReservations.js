import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { useNavigate } from 'react-router-dom';

function PatientReservations() {
  const [reservations, setReservations] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const formatIst = (dateValue) => {
    if (!dateValue) return '-';
    const date = new Date(dateValue);
    return date.toLocaleString('en-IN', {
      timeZone: 'Asia/Kolkata',
      year: 'numeric',
      month: 'long',
      day: 'numeric',
      hour: '2-digit',
      minute: '2-digit',
      hour12: false
    });
  };

  const fetchReservations = async () => {
    const patientId = localStorage.getItem('patientId');
    if (!patientId) {
      navigate('/');
      return;
    }

    try {
      const res = await axios.get(`/api/patients/${patientId}/reservations`);
      setReservations(res.data.reservations || []);
    } catch (err) {
      console.error('Reservations fetch error', err);
      setError('Failed to load reservations');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchReservations();
  }, [navigate]);

  const cancelReservation = async (reservationId) => {
    try {
      const patientId = localStorage.getItem('patientId');
      await axios.put(`/api/patient/reservations/${reservationId}/cancel`, { patient_id: patientId });
      fetchReservations();
    } catch (err) {
      console.error('Cancel reservation error', err);
      setError('Failed to cancel reservation');
    }
  };

  if (loading) return <div className="form-container">Loading reservations...</div>;
  if (error) return <div className="form-container"><div className="error">{error}</div></div>;

  return (
    <div className="form-container">
      <h2>Your Active Reservations</h2>
      {reservations.length === 0 ? (
        <div>
          <p>You have no active reservations.</p>
          <button className="btn" onClick={() => navigate('/patient/request')}>Make a Request</button>
        </div>
      ) : (
        <div>
          {reservations.map(r => (
            <div key={r.reservation_id} style={{ border: '1px solid #ddd', padding: '10px', marginBottom: '8px' }}>
              <p><strong>Reservation ID:</strong> {r.reservation_id}</p>
              <p><strong>Hospital ID:</strong> {r.hospital_id}</p>
              <p><strong>Bed ID:</strong> {r.bed_id}</p>
              <p><strong>Booked until:</strong> {formatIst(r.reserved_until)} (IST)</p>
              <p><strong>Status:</strong> {r.status}</p>
              <button onClick={() => cancelReservation(r.reservation_id)} className="btn btn-secondary" style={{ marginTop: '10px' }}>
                Cancel Reservation
              </button>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

export default PatientReservations;
