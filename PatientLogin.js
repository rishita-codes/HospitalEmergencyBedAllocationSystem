import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';

function PatientLogin() {
  const [phoneNumber, setPhoneNumber] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    setError('');

    try {
      // Validate phone number: must be exactly 10 digits
      if (!/^\d{10}$/.test(phoneNumber)) {
        setError('Phone number must be exactly 10 digits');
        setLoading(false);
        return;
      }
      const response = await axios.post('/api/auth/patient-login', {
        phone_number: phoneNumber
      });

      // Store patient info in localStorage
      localStorage.setItem('patientPhone', phoneNumber);
      if (response.data.patient_id) {
        localStorage.setItem('patientId', response.data.patient_id);
        localStorage.setItem('patientName', response.data.name);
      }

      // If patient has active reservations, go to reservations list
      const pid = response.data.patient_id;
      if (pid) {
        try {
          const res = await axios.get(`/api/patients/${pid}/reservations`);
          if (res.data.reservations && res.data.reservations.length > 0) {
            navigate('/patient/reservations');
            return;
          }
        } catch (e) {
          // ignore, fall through to request form
        }
      }

      // Navigate to request form
      navigate('/patient/request');
    } catch (error) {
      setError('Login failed. Please try again.');
      console.error('Login error:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="form-container">
      <h2>Patient Portal</h2>
      <p>Enter your phone number to access the bed reservation system</p>

      <form onSubmit={handleSubmit}>
        <div className="form-group">
          <label htmlFor="phone">Phone Number:</label>
          <input
            type="tel"
            id="phone"
            value={phoneNumber}
            onChange={(e) => setPhoneNumber(e.target.value)}
            placeholder="Enter your phone number"
            required
          />
        </div>

        <button type="submit" className="btn" disabled={loading}>
          {loading ? 'Logging in...' : 'Continue'}
        </button>
      </form>

      {error && <div className="error">{error}</div>}

      <div style={{ marginTop: '20px' }}>
        <a href="/staff/login" className="btn btn-secondary">Staff Login</a>
      </div>
    </div>
  );
}

export default PatientLogin;