import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';

function ReservationConfirmation() {
  const [timeLeft, setTimeLeft] = useState(30 * 60); // 30 minutes in seconds
  const navigate = useNavigate();

  const reservationId = localStorage.getItem('reservationId');
  const bedId = localStorage.getItem('bedId');
  const hospitalName = localStorage.getItem('hospitalName');
  const reservedUntil = localStorage.getItem('reservedUntil');

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

  useEffect(() => {
    if (!reservationId) {
      navigate('/');
      return;
    }

    const timer = setInterval(() => {
      setTimeLeft(prev => {
        if (prev <= 1) {
          clearInterval(timer);
          return 0;
        }
        return prev - 1;
      });
    }, 1000);

    return () => clearInterval(timer);
  }, [reservationId, navigate]);

  const formatTime = (seconds) => {
    const mins = Math.floor(seconds / 60);
    const secs = seconds % 60;
    return `${mins}:${secs.toString().padStart(2, '0')}`;
  };

  const handleNewRequest = () => {
    // Clear reservation data
    localStorage.removeItem('reservationId');
    localStorage.removeItem('bedId');
    localStorage.removeItem('hospitalName');
    localStorage.removeItem('reservedUntil');
    localStorage.removeItem('requestId');

    navigate('/patient/request');
  };

  const handleCancelReservation = async () => {
    try {
      const patientId = localStorage.getItem('patientId');
      await axios.put(`/api/patient/reservations/${reservationId}/cancel`, { patient_id: patientId });
      localStorage.removeItem('reservationId');
      localStorage.removeItem('bedId');
      localStorage.removeItem('hospitalName');
      localStorage.removeItem('reservedUntil');
      localStorage.removeItem('requestId');
      navigate('/patient/reservations');
    } catch (error) {
      console.error('Cancel reservation error:', error);
      alert('Failed to cancel reservation. Please try again.');
    }
  };

  if (timeLeft === 0) {
    return (
      <div className="form-container">
        <h2>Reservation Expired</h2>
        <p>Your bed reservation has expired. Please make a new request.</p>
        <button onClick={handleNewRequest} className="btn">
          Make New Request
        </button>
      </div>
    );
  }

  return (
    <div className="form-container">
      <h2>Bed Reserved Successfully!</h2>

      <div style={{ margin: '20px 0', padding: '20px', backgroundColor: '#e8f5e8', borderRadius: '8px' }}>
        <h3>Reservation Details</h3>
        <p><strong>Hospital:</strong> {hospitalName}</p>
        <p><strong>Bed ID:</strong> {bedId}</p>
        <p><strong>Reservation ID:</strong> {reservationId}</p>
        <p><strong>Booked until:</strong> {formatIst(reservedUntil)} (IST)</p>
      </div>

      <div className="timer">
        Time Remaining: {formatTime(timeLeft)}
      </div>

      <div style={{ margin: '20px 0', padding: '15px', backgroundColor: '#fff3cd', borderRadius: '8px' }}>
        <h4>Important Instructions:</h4>
        <ul style={{ textAlign: 'left' }}>
          <li>Arrive at the hospital within the reservation time</li>
          <li>Bring your ID and this reservation confirmation</li>
          <li>Contact the hospital directly if you have questions</li>
          <li>The reservation will expire automatically after 30 minutes</li>
        </ul>
      </div>

      <button onClick={handleCancelReservation} className="btn btn-secondary" style={{ marginRight: '10px' }}>
        Cancel Reservation
      </button>
      <button onClick={handleNewRequest} className="btn btn-secondary">
        Make Another Request
      </button>
    </div>
  );
}

export default ReservationConfirmation;