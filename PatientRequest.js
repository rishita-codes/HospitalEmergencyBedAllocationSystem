import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';

function PatientRequest() {
  const [formData, setFormData] = useState({
    name: '',
    age: '',
    symptoms: '',
    esi_level: '',
    desired_bed_type: '2' // Default to general
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const [cities, setCities] = useState([]);
  const [selectedCity, setSelectedCity] = useState('');
  const navigate = useNavigate();

  useEffect(() => {
    // Pre-fill name if we have it from login
    const patientName = localStorage.getItem('patientName');
    if (patientName) {
      setFormData(prev => ({ ...prev, name: patientName }));
    }
    // load cities for dropdown
    axios.get('/api/hospitals/cities').then(res => {
      setCities(res.data.cities || []);
      if (res.data.cities && res.data.cities.length > 0) {
        setSelectedCity(res.data.cities[0].city);
      }
    }).catch(() => {});
  }, []);

  const handleChange = (e) => {
    const { name, value } = e.target;
    setFormData(prev => ({ ...prev, [name]: value }));
  };

  const determineESILevel = (symptoms) => {
    // Simple ESI determination based on symptoms
    const criticalSymptoms = ['chest pain', 'difficulty breathing', 'unconscious', 'severe bleeding'];
    const urgentSymptoms = ['moderate pain', 'fever', 'vomiting', 'dehydration'];

    const lowerSymptoms = symptoms.toLowerCase();

    if (criticalSymptoms.some(symptom => lowerSymptoms.includes(symptom))) {
      return 1; // ESI 1 - Most urgent
    } else if (urgentSymptoms.some(symptom => lowerSymptoms.includes(symptom))) {
      return 2; // ESI 2 - Urgent
    } else {
      return 3; // ESI 3 - Less urgent
    }
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setError('');

    // Validate age (must not be greater than 130)
    const ageNum = parseInt(formData.age, 10);
    if (isNaN(ageNum) || ageNum < 0) {
      setError('Please enter a valid age');
      return;
    }
    if (ageNum > 130) {
      setError('Age must not be greater than 130');
      return;
    }

    // Validate phone number stored from login
    const phone = localStorage.getItem('patientPhone') || '';
    if (!/^\d{10}$/.test(phone)) {
      setError('Phone number must be exactly 10 digits');
      return;
    }

    setLoading(true);

    try {
      // Auto-determine ESI level from symptoms
      const esiLevel = determineESILevel(formData.symptoms);

      const requestData = {
        phone_number: localStorage.getItem('patientPhone'),
        name: formData.name,
        age: ageNum,
        esi_level: esiLevel,
        desired_bed_type: parseInt(formData.desired_bed_type)
      };

      const response = await axios.post('/api/patient/requests', requestData);

      // Store request info
      localStorage.setItem('requestId', response.data.request_id);
      localStorage.setItem('patientId', response.data.patient_id);
      // store selected city for availability lookup
      if (selectedCity) localStorage.setItem('patientCity', selectedCity);

      // Navigate to hospital selection
      navigate('/patient/hospitals');
    } catch (error) {
      setError('Failed to submit request. Please try again.');
      console.error('Request error:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="form-container">
      <h2>Emergency Bed Request</h2>
      <p>Please provide your information to determine bed priority</p>

      <form onSubmit={handleSubmit}>
        <div className="form-group">
          <label htmlFor="name">Full Name:</label>
          <input
            type="text"
            id="name"
            name="name"
            value={formData.name}
            onChange={handleChange}
            required
          />
        </div>

        <div className="form-group">
          <label htmlFor="age">Age:</label>
          <input
            type="number"
            id="age"
            name="age"
            value={formData.age}
            onChange={handleChange}
            min="0"
            max="150"
            required
          />
        </div>

        <div className="form-group">
          <label htmlFor="symptoms">Symptoms/Condition:</label>
          <textarea
            id="symptoms"
            name="symptoms"
            value={formData.symptoms}
            onChange={handleChange}
            placeholder="Describe your symptoms (e.g., chest pain, difficulty breathing, fever, etc.)"
            rows="4"
            required
          />
        </div>

        <div className="form-group">
          <label htmlFor="desired_bed_type">Preferred Bed Type:</label>
          <select
            id="desired_bed_type"
            name="desired_bed_type"
            value={formData.desired_bed_type}
            onChange={handleChange}
          >
            <option value="0">ICU (Intensive Care Unit)</option>
            <option value="1">Emergency</option>
            <option value="2">General</option>
            <option value="3">Specialty</option>
          </select>
        </div>

        <div className="form-group">
          <label htmlFor="city">Your City:</label>
          <select id="city" name="city" value={selectedCity} onChange={(e) => setSelectedCity(e.target.value)}>
            {cities.map(c => (
              <option key={c.city} value={c.city}>{c.city}</option>
            ))}
          </select>
        </div>

        <button type="submit" className="btn" disabled={loading}>
          {loading ? 'Submitting...' : 'Submit Request'}
        </button>
      </form>

      {error && <div className="error">{error}</div>}

      <div style={{ marginTop: '20px', fontSize: '0.9rem', color: '#666' }}>
        <p><strong>ESI Level Determination:</strong></p>
        <ul style={{ textAlign: 'left' }}>
          <li>ESI 1: Critical conditions (chest pain, severe bleeding, unconsciousness)</li>
          <li>ESI 2: Urgent conditions (moderate pain, high fever, dehydration)</li>
          <li>ESI 3: Less urgent conditions (minor injuries, stable chronic conditions)</li>
        </ul>
      </div>
    </div>
  );
}

export default PatientRequest;