function decodeUplink(input) {
  let dist = (input.bytes[0] << 8) | input.bytes[1];
  let temp = (input.bytes[2] << 8) | input.bytes[3];

  return {
    data: {
      distance_m: dist / 100,
      temperature: temp / 100
    }
  };
}