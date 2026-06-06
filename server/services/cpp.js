const { spawn } = require('child_process');
const path = require('path');
const { parseAllocatorAssignments } = require('../utils/allocatorParser');

function runAllocator(algorithm, hospitalId) {
  return new Promise((resolve, reject) => {
    const allocatorPath = path.join(__dirname, '..', '..', 'cpp', 'hospital_allocator');
    const env = Object.assign({}, process.env);
    const cpp = spawn(allocatorPath, [algorithm, String(hospitalId)], { env });

    let stdout = '';
    let stderr = '';

    cpp.stdout.on('data', (data) => {
      stdout += data.toString();
    });

    cpp.stderr.on('data', (data) => {
      stderr += data.toString();
    });

    cpp.on('close', (code) => {
      if (code !== 0) {
        return reject(new Error(`Allocator failed: ${stderr.trim()}`));
      }
      try {
        const assignments = parseAllocatorAssignments(stdout);
        resolve(assignments);
      } catch (error) {
        reject(error);
      }
    });

    cpp.on('error', (error) => {
      reject(error);
    });
  });
}

module.exports = { runAllocator };
