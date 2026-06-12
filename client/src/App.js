import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import axios from 'axios';
import './App.css';

// Components
import PatientLogin from './components/PatientLogin';
import PatientRequest from './components/PatientRequest';
import HospitalSelection from './components/HospitalSelection';
import ReservationConfirmation from './components/ReservationConfirmation';
import PatientReservations from './components/PatientReservations';
import StaffLogin from './components/StaffLogin';
import CoordinatorDashboard from './components/CoordinatorDashboard';
import NurseDashboard from './components/NurseDashboard';

function App() {
  const authToken = localStorage.getItem('authToken');
  if (authToken) {
    axios.defaults.headers.common['Authorization'] = `Bearer ${authToken}`;
  }

  return (
    <Router>
      <div className="App">
        <header className="App-header">
          <h1>Hospital Emergency Bed Allocation System</h1>
          {/* Hospital name intentionally not shown in global header */}
        </header>
        <main>
          <Routes>
            <Route path="/" element={<PatientLogin />} />
            <Route path="/patient/request" element={<PatientRequest />} />
            <Route path="/patient/hospitals" element={<HospitalSelection />} />
            <Route path="/patient/reservation" element={<ReservationConfirmation />} />
            <Route path="/patient/reservations" element={<PatientReservations />} />
            <Route path="/staff/login" element={<StaffLogin />} />
            <Route path="/coordinator/dashboard" element={<CoordinatorDashboard />} />
            <Route path="/nurse/dashboard" element={<NurseDashboard />} />
          </Routes>
        </main>
      </div>
    </Router>
  );
}

export default App;
