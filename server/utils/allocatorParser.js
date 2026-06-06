function parseAllocatorAssignments(rawOutput) {
  const output = (rawOutput || '').trim();
  const jsonStart = output.indexOf('[');
  const jsonEnd = output.lastIndexOf(']');
  if (jsonStart === -1 || jsonEnd === -1 || jsonEnd < jsonStart) {
    throw new Error('Allocator JSON payload not found');
  }
  const jsonPayload = output.slice(jsonStart, jsonEnd + 1);
  const parsed = JSON.parse(jsonPayload);
  if (!Array.isArray(parsed)) {
    throw new Error('Allocator output must be an array');
  }
  return parsed;
}

module.exports = { parseAllocatorAssignments };
