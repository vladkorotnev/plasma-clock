export const BAUD_RATE = 705600;
export const BLOCK_BYTES = 512;
export const BLOCK_BITS = BLOCK_BYTES * 8;
export const CHANNEL_COUNT = 6;
// -15 dB relative to the original 0.8 level, calibrated against the default HD mix.
export const DEVICE_GAIN = 0.8 * 10 ** (-15 / 20);
