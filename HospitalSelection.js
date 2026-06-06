import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';

function HospitalSelection() {
  const [hospitals, setHospitals] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const [fallback, setFallback] = useState(false);
  const navigate = useNavigate();

  useEffect(() => {
    const fetchAvailableHospitals = async () => {
      setLoading(true);
      try {
        const requestId = localStorage.getItem('requestId');
        const city = localStorage.getItem('patientCity');
        const q = city ? `?city=${encodeURIComponent(city)}` : '';
        const res = await axios.get(`/api/patient/requests/${requestId}/availability${q}`);
        setHospitals(res.data.hospitals || []);
        setFallback(res.data.fallback || false);
      } catch (e) {
        console.error('Fetch hospitals error:', e);
        setError('Failed to load available hospitals.');
      } finally {
        setLoading(false);
      }
    };
    fetchAvailableHospitals();
  }, []);

  const handleReserve = async (hospital) => {
    try {
      const requestId = localStorage.getItem('requestId');
      const patientId = localStorage.getItem('patientId');
      const res = await axios.post('/api/patient/reservations', {
        request_id: requestId,
        patient_id: patientId,
        hospital_id: hospital.hospital_id,
      });

      localStorage.setItem('reservationId', res.data.reservation_id);
      localStorage.setItem('bedId', res.data.bed_id);
      localStorage.setItem('hospitalName', hospital.name);
      localStorage.setItem('reservedUntil', res.data.reserved_until);
      navigate('/patient/reservation');
    } catch (e) {
      console.error('Reservation error:', e);
      setError('Failed to create reservation. Please try again.');
    }
  };

  if (loading) return <div className="form-container">Loading available hospitals...</div>;
  if (error) {
    return (
      <div className="form-container">
        <div className="error">{error}</div>
        <button onClick={() => window.location.reload()} className="btn">
          Retry
        </button>
      </div>
    );
  }

  return (
    <div>
      <h2>Available Hospitals</h2>
      <p>Select a hospital to reserve a bed (30-minute reservation)</p>

      {hospitals.length === 0 ? (
        <div className="form-container">
          <p>No hospitals with available beds found at this time.</p>
          <button onClick={() => navigate('/patient/request')} className="btn">
            Back to Request Form
          </button>
        </div>
      ) : (
        <>
          {fallback && (
            <div className="info" style={{ marginBottom: 16 }}>
              No beds of the requested type were available, showing other available beds instead.
            </div>
          )}

          <div className="hospital-list">
            {hospitals.map((hospital) => (
              <div key={hospital.hospital_id} className="hospital-card">
                <h3>{hospital.name}</h3>
                <p>
                  <strong>Available Beds:</strong> {hospital.available_count}
                </p>
                {hospital.distance_km != null && (
                  <p>
                    <strong>Distance:</strong> {hospital.distance_km.toFixed(1)} km
                  </p>
                )}
                <button onClick={() => handleReserve(hospital)} className="btn">
                  Reserve Bed Here
                </button>
              </div>
            ))}
          </div>
        </>
      )}

      <div style={{ marginTop: 20 }}>
        <button onClick={() => navigate('/patient/request')} className="btn btn-secondary">
          Back to Request Form
        </button>
      </div>
    </div>
  );
}

export default HospitalSelection;