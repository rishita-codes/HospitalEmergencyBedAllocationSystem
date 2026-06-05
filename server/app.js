const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const cron = require('node-cron');
const authRoutes = require('./routes/auth');
const patientRoutes = require('./routes/patients');
const hospitalRoutes = require('./routes/hospitals');
const allocationRoutes = require('./routes/allocation');
const bedRoutes = require('./routes/beds');
const reservationRoutes = require('./routes/reservations');
const errorHandler = require('./middleware/errorHandler');
const { expireReservations } = require('./services/reservations');

const app = express();

app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

app.use('/api/auth', authRoutes);
app.use('/api', patientRoutes);
app.use('/api', hospitalRoutes);
app.use('/api', allocationRoutes);
app.use('/api', bedRoutes);
app.use('/api', reservationRoutes);

app.use((req, res) => {
  res.status(404).json({ error: 'Not found' });
});

app.use(errorHandler);

cron.schedule('* * * * *', async () => {
  try {
    await expireReservations();
    console.log('Expired old reservations');
  } catch (error) {
    console.error('Error expiring reservations:', error);
  }
});

module.exports = app;
