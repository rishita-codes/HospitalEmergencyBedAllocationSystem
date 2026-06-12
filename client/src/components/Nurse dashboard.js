import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';

function NurseDashboard() {
  const [beds, setBeds] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const hospitalId = localStorage.getItem('hospitalId');
  const hospitalName = localStorage.getItem('staffHospitalName');

  useEffect(() => {
    if (!hospitalId) {
      navigate('/staff/login');
      return;
    }
    fetchBeds();
  }, [hospitalId, navigate]);

  const formatIst = (ts) => {
    if (!ts) return '-';
    return `${new Date(ts).toLocaleString('en-IN', { timeZone: 'Asia/Kolkata' })} IST`;
  };

  const fetchBeds = async () => {
    try {
      const response = await axios.get(`/api/hospitals/${hospitalId}/beds`);
      setBeds(response.data.beds);
    } catch (error) {
      setError('Failed to load bed data');
      console.error('Beds error:', error);
    } finally {
      setLoading(false);
    }
  };

  const updateBedStatus = async (bedId, isOccupied) => {
    try {
      if (!isOccupied) {
        await axios.put(`/api/beds/${bedId}/free`);
      }
      await fetchBeds(); // Refresh data
    } catch (error) {
      setError('Failed to update bed status');
      console.error('Update bed error:', error);
    }
  };

  const logout = () => {
    localStorage.clear();
    navigate('/staff/login');
  };

  if (loading) {
    return <div className="form-container">Loading dashboard...</div>;
  }

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '20px' }}>
        <div className="dashboard-title">
          <h2>Nurse Dashboard</h2>
          {hospitalName && <div className="dashboard-hospital-name">{hospitalName}</div>}
        </div>
        <button onClick={logout} className="btn btn-secondary">Logout</button>
      </div>

      {error && <div className="error">{error}</div>}

      <div>
        <h3>Bed Management</h3>
        <div className="bed-grid">
          {beds.map((bed) => (
            <div
              key={bed.bed_id}
              className={`bed-item ${bed.is_occupied ? 'bed-occupied' : 'bed-available'}`}
            >
              <div><strong>{bed.bed_id}</strong></div>
              <div>Type: {['ICU', 'Emergency', 'General', 'Specialty'][bed.type]}</div>
              <div>
                Status: {bed.is_occupied ? 'Occupied' : (bed.reservation_id ? 'Reserved' : 'Available')}
              </div>
              {bed.assigned_patient_id && (
                <div>Patient: {bed.assigned_patient_id}</div>
              )}
              {bed.reservation_id && !bed.is_occupied && (
                <div style={{ marginTop: '6px', fontSize: '12px' }}>
                  <div><strong>Reservation ID:</strong> {bed.reservation_id}</div>
                  <div><strong>Reserved Until:</strong> {formatIst(bed.reserved_until)}</div>
                  <div><strong>Time Left:</strong> {bed.reservation_seconds_left ?? 0}s</div>
                </div>
              )}

              <div style={{ marginTop: '10px' }}>
                {bed.is_occupied ? (
                  <button
                    onClick={() => updateBedStatus(bed.bed_id, false)}
                    className="btn"
                    style={{ fontSize: '12px', padding: '5px 10px' }}
                  >
                    Discharge (Mark Available)
                  </button>
                ) : (
                  <div style={{ fontSize: '12px', color: '#666' }}>Nurse cannot assign beds.</div>
                )}
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}

export default NurseDashboard;
