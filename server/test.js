const axios = require('axios');

const BASE_URL = 'http://localhost:3001';

async function testAPI() {
  try {
    console.log('Testing Hospital EBAS API...');

    // Test patient login
    console.log('1. Testing patient login...');
    const patientLogin = await axios.post(`${BASE_URL}/api/auth/patient-login`, {
      phone_number: '1234567890'
    });
    console.log('✓ Patient login:', patientLogin.data);

    // Test staff login
    console.log('2. Testing staff login...');
    const staffLogin = await axios.post(`${BASE_URL}/api/auth/staff-login`, {
      username: 'coord1',
      password: 'pass123'
    });
    console.log('✓ Staff login:', staffLogin.data);

    console.log('All tests passed! 🎉');

  } catch (error) {
    console.error('Test failed:', error.response?.data || error.message);
  }
}

// Run test if called directly
if (require.main === module) {
  testAPI();
}

module.exports = { testAPI };