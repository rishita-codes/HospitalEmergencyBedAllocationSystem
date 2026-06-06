import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import axios from 'axios';

function StaffLogin() {
  const [credentials, setCredentials] = useState({
    username: '',
    password: ''
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');
  const navigate = useNavigate();

  const handleChange = (e) => {
    const { name, value } = e.target;
    setCredentials(prev => ({ ...prev, [name]: value }));
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    setError('');

    try {
      const response = await axios.post('/api/auth/staff-login', credentials);

      // Store user info and auth token
      localStorage.setItem('userId', response.data.user_id);
      localStorage.setItem('userRole', response.data.role);
      localStorage.setItem('hospitalId', response.data.hospital_id);
      // store staff-specific hospital name separately so patient flows don't affect staff UI
      localStorage.setItem('staffHospitalName', response.data.hospital_name || '');
      localStorage.setItem('authToken', response.data.token);
      axios.defaults.headers.common['Authorization'] = `Bearer ${response.data.token}`;

      // Navigate based on role
      if (response.data.role === 'coordinator') {
        navigate('/coordinator/dashboard');
      } else if (response.data.role === 'nurse') {
        navigate('/nurse/dashboard');
      } else {
        setError('Unauthorized role');
      }
    } catch (error) {
      setError('Invalid username or password');
      console.error('Login error:', error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="form-container">
      <h2>Staff Login</h2>
      <p>Login to access the hospital management system</p>

      <form onSubmit={handleSubmit}>
        <div className="form-group">
          <label htmlFor="username">Username:</label>
          <input
            type="text"
            id="username"
            name="username"
            value={credentials.username}
            onChange={handleChange}
            required
          />
        </div>

        <div className="form-group">
          <label htmlFor="password">Password:</label>
          <input
            type="password"
            id="password"
            name="password"
            value={credentials.password}
            onChange={handleChange}
            required
          />
        </div>

        <button type="submit" className="btn" disabled={loading}>
          {loading ? 'Logging in...' : 'Login'}
        </button>
      </form>

      {error && <div className="error">{error}</div>}

      <div style={{ marginTop: '20px' }}>
        <a href="/" className="btn btn-secondary">Patient Portal</a>
      </div>
    </div>
  );
}

export default StaffLogin;