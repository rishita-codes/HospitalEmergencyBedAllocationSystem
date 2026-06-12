import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';

function CoordinatorDashboard() {
  const [beds, setBeds] = useState([]);
  const [patients, setPatients] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const [previewAssignments, setPreviewAssignments] = useState([]);
  const [previewAlgorithm, setPreviewAlgorithm] = useState(null);
  const navigate = useNavigate();
  const hospitalId = localStorage.getItem('hospitalId');
  const [newPatient, setNewPatient] = useState({
    name: '',
    phone_number: '',
    age: '',
    esi_level: '3',
    desired_bed_type: '1',
    preferred_hospital_id: hospitalId || ''
  });
  const [adding, setAdding] = useState(false);
  const [addError, setAddError] = useState('');
  const [showAddModal, setShowAddModal] = useState(false);

  const formatIst = (ts) => {
    if (!ts) return '-';
    return `${new Date(ts).toLocaleString('en-IN', { timeZone: 'Asia/Kolkata' })} IST`;
  };


  useEffect(() => {
    if (!hospitalId) {
      navigate('/staff/login');
      return;
    }
    fetchData();
  }, [hospitalId, navigate]);

  const openAddModal = () => {
    setNewPatient(prev => ({ ...prev, preferred_hospital_id: hospitalId }));
    setAddError('');
    setShowAddModal(true);
  };

  const closeAddModal = () => {
    setShowAddModal(false);
  };

  const handleNewChange = (e) => {
    const { name, value } = e.target;
    setNewPatient(prev => ({ ...prev, [name]: value }));
  };

  const submitAddPatient = async (e) => {
    e.preventDefault();
    setAddError('');
    if (!newPatient.name || !newPatient.phone_number) {
      setAddError('Name and phone number are required');
      return;
    }
    // Validate phone number (must be exactly 10 digits)
    if (!/^\d{10}$/.test(newPatient.phone_number)) {
      setAddError('Phone number must be exactly 10 digits');
      return;
    }
    // Validate age if provided
    if (newPatient.age) {
      const ageNum = parseInt(newPatient.age, 10);
      if (isNaN(ageNum) || ageNum < 0) {
        setAddError('Please enter a valid age');
        return;
      }
      if (ageNum > 130) {
        setAddError('Age must not be greater than 130');
        return;
      }
    }
    setAdding(true);
    try {
      const payload = {
        name: newPatient.name,
        phone_number: newPatient.phone_number,
        age: newPatient.age ? parseInt(newPatient.age, 10) : null,
        esi_level: newPatient.esi_level ? parseInt(newPatient.esi_level, 10) : null,
        desired_bed_type: newPatient.desired_bed_type ? parseInt(newPatient.desired_bed_type, 10) : null,
        preferred_hospital_id: newPatient.preferred_hospital_id || hospitalId
      };

      const res = await axios.post(`/api/hospitals/${hospitalId}/waiting-patients`, payload);
      // success
      setShowAddModal(false);
      setNewPatient({ name: '', phone_number: '', age: '', esi_level: '3', desired_bed_type: '1', preferred_hospital_id: hospitalId });
      await fetchData();
    } catch (err) {
      console.error('Add waiting patient failed', err);
      setAddError(err?.response?.data?.error || 'Failed to add patient');
    } finally {
      setAdding(false);
    }
  };

  const fetchData = async () => {
    try {
      const [bedsResponse, patientsResponse] = await Promise.all([
        axios.get(`/api/hospitals/${hospitalId}/beds`),
        axios.get(`/api/hospitals/${hospitalId}/patients/waiting`)
      ]);

      setBeds(bedsResponse.data.beds);
      setPatients(patientsResponse.data.patients);
    } catch (error) {
      setError('Failed to load dashboard data');
      console.error('Dashboard error:', error);
    } finally {
      setLoading(false);
    }
  };

  const runAllocation = async (algorithm) => {
    try {
      await axios.post(`/api/allocate/${algorithm}`, { hospital_id: hospitalId });
      // Refresh data after allocation
      await fetchData();
    } catch (error) {
      setError(`Failed to run ${algorithm} allocation`);
      console.error('Allocation error:', error);
    }
  };

  const previewAllocation = async (algorithm) => {
    try {
      setError('');
      const res = await axios.post(`/api/allocate/${algorithm}/preview`, { hospital_id: hospitalId });
      setPreviewAssignments(res.data.assignments || []);
      setPreviewAlgorithm(algorithm);
    } catch (err) {
      console.error('Preview error', err);
      setError('Failed to preview allocation');
    }
  };

  const applyPreview = async () => {
    if (!previewAlgorithm) return;
    try {
      await axios.post(`/api/allocate/${previewAlgorithm}`, { hospital_id: hospitalId });
      setPreviewAssignments([]);
      setPreviewAlgorithm(null);
      await fetchData();
    } catch (err) {
      console.error('Apply preview error', err);
      setError('Failed to apply allocation');
    }
  };

  const markReservationArrived = async (reservationId) => {
    try {
      setError('');
      await axios.put(`/api/reservations/${reservationId}/arrive`);
      await fetchData();
    } catch (err) {
      console.error('Mark arrived error', err);
      setError(err?.response?.data?.error || 'Failed to mark reservation as arrived');
    }
  };

  const logout = () => {
    localStorage.clear();
    navigate('/staff/login');
  };

  if (loading) {
    return <div className="form-container">Loading dashboard...</div>;
  }

  const hospitalName = localStorage.getItem('staffHospitalName');

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '20px' }}>
        <div className="dashboard-title">
          <h2>Coordinator Dashboard</h2>
          {hospitalName && <div className="dashboard-hospital-name">{hospitalName}</div>}
        </div>
        <div>
          <button onClick={openAddModal} className="btn" style={{ marginRight: '8px' }}>Add Waiting Patient</button>
          <button onClick={logout} className="btn btn-secondary">Logout</button>
        </div>
      </div>

      {error && <div className="error">{error}</div>}

      <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '20px' }}>
        {/* Patient Queue */}
        <div>
          <h3>Waiting Patients ({patients.length})</h3>
          <div className="patient-queue">
            {patients.length === 0 ? (
              <p>No waiting patients</p>
            ) : (
              patients.map((patient) => (
                <div key={patient.patient_id} className="patient-item">
                  <div>
                    <strong>{patient.name}</strong> (ID: {patient.patient_id})
                    <br />
                    Age: {patient.age}, ESI: {patient.esi_level}
                  </div>
                  <div>
                    {new Date(patient.created_at).toLocaleString()}
                  </div>
                </div>
              ))
            )}
          </div>

          <div style={{ marginTop: '20px' }}>
            <button onClick={() => previewAllocation('greedy')} className="btn">
              Preview Greedy Allocation
            </button>
            <button onClick={() => previewAllocation('optimal')} className="btn" style={{ marginLeft: '10px' }}>
              Preview Optimal Allocation
            </button>
            <button onClick={applyPreview} className="btn" style={{ marginLeft: '10px' }} disabled={!previewAlgorithm}>
              Apply Preview ({previewAlgorithm || 'none'})
            </button>
          </div>
        </div>

        {/* Bed Status */}
        <div>
          <h3>Bed Status ({beds.length})</h3>
          <div className="bed-grid">
            {beds.map((bed) => (
              <div
                key={bed.bed_id}
                className={`bed-item ${bed.is_occupied ? 'bed-occupied' : 'bed-available'}`}
              >
                <div><strong>{bed.bed_id}</strong></div>
                <div>Type: {['ICU', 'Emergency', 'General', 'Specialty'][bed.type]}</div>
                <div>
                  {bed.is_occupied ? 'Occupied' : (bed.reservation_id ? 'Reserved' : 'Available')}
                </div>
                {bed.assigned_patient_id && (
                  <div>Patient: {bed.assigned_patient_id}</div>
                )}
                {bed.reservation_id && !bed.is_occupied && (
                  <div style={{ marginTop: '6px', fontSize: '12px' }}>
                    <div><strong>Reservation ID:</strong> {bed.reservation_id}</div>
                    <div><strong>Time reserved until this time (IST):</strong> {formatIst(bed.reserved_until)}</div>
                    <div style={{ marginTop: '6px', fontSize: '12px', color: '#555' }}>
                      This bed is reserved and waiting for patient arrival. Confirm assignment when the reserved patient arrives.
                    </div>
                    <button
                      className="btn"
                      style={{ marginTop: '6px', fontSize: '12px', padding: '5px 8px' }}
                      onClick={() => markReservationArrived(bed.reservation_id)}
                    >
                      Confirm Assign
                    </button>
                  </div>
                )}
              </div>
            ))}
          </div>
        </div>
      </div>

      {previewAssignments && previewAssignments.length > 0 && (
        <div style={{ marginTop: '20px' }}>
          <h3>Preview Assignments ({previewAlgorithm})</h3>
          <div>
            {previewAssignments.map((a, idx) => (
              <div key={idx} style={{ border: '1px solid #ddd', padding: '8px', marginBottom: '6px' }}>
                <div><strong>Patient ID:</strong> {a.patient_id}</div>
                <div><strong>Bed ID:</strong> {a.bed_id}</div>
              </div>
            ))}
          </div>
        </div>
      )}

      {showAddModal && (
        <div style={{ position: 'fixed', top: 0, left: 0, right: 0, bottom: 0, background: 'rgba(0,0,0,0.4)', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
          <div style={{ background: '#fff', padding: '20px', borderRadius: '8px', width: '420px' }}>
            <h3>Add Waiting Patient</h3>
            <form onSubmit={submitAddPatient}>
              <div className="form-group">
                <label>Name</label>
                <input name="name" value={newPatient.name} onChange={handleNewChange} required />
              </div>
              <div className="form-group">
                <label>Phone number</label>
                <input name="phone_number" value={newPatient.phone_number} onChange={handleNewChange} required />
              </div>
              <div className="form-group">
                <label>Age</label>
                <input name="age" value={newPatient.age} onChange={handleNewChange} />
              </div>
              <div className="form-group">
                <label>ESI level</label>
                <select name="esi_level" value={newPatient.esi_level} onChange={handleNewChange}>
                  <option value="1">1</option>
                  <option value="2">2</option>
                  <option value="3">3</option>
                </select>
              </div>
              <div className="form-group">
                <label>Desired bed type</label>
                <select name="desired_bed_type" value={newPatient.desired_bed_type} onChange={handleNewChange}>
                  <option value="0">ICU</option>
                  <option value="1">Emergency</option>
                  <option value="2">General</option>
                  <option value="3">Specialty</option>
                </select>
              </div>
              <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '8px' }}>
                <button type="button" className="btn btn-secondary" onClick={closeAddModal}>Cancel</button>
                <button type="submit" className="btn" disabled={adding}>{adding ? 'Adding...' : 'Add'}</button>
              </div>
              {addError && <div className="error" style={{ marginTop: '8px' }}>{addError}</div>}
            </form>
          </div>
        </div>
      )}
    </div>
  );
}

export default CoordinatorDashboard;
