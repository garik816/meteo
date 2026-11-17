# Meteo v2 protocol

Wire format (one newline-terminated ASCII frame per reading):

    $M2,1,42,235,564,7608,4021*XX

XX is the calculated uppercase 2-digit CRC-8 value (polynomial 0x07, init 0, xorout 0, non-reflected). CRC covers ASCII bytes after $ and before *.

Fields: M2 version; 1 node ID; 42 rolling 16-bit sequence; 235 = 23.5 degrees C; 564 = 56.4 percent RH; 7608 = 760.8 mmHg; 4021 = 4021 mV battery voltage.

Valid temperature -40..85 degrees C; humidity 0..100%; pressure 300..900 mmHg; voltage 1800..6000 mV. A receiver accepts frames only after CRC and range validation, rejects partial/oversized frames, and resynchronizes at the next $ marker. After 180 seconds with no valid frame, the receiver declares the remote data stale. Protocol offers integrity detection, but **no encryption, authentication or retransmission**. Legacy C01,T=... messages are incompatible: upgrade both devices together.
