# EnviroSense AQMS Data Samples

This file contains sample gas measurement data from the EnviroSense Air Quality Monitoring System (AQMS).

## Sensor Parameters

The system monitors the following gas parameters:
- CO2: Carbon Dioxide (ppm)
- SO2: Sulfur Dioxide (ppm)
- H2S: Hydrogen Sulfide (ppm)
- CH4: Methane (ppm)
- NO2: Nitrogen Dioxide (ppm)
- C2H5OH: Ethanol (ppm)
- H2: Hydrogen (ppm)
- NH3: Ammonia (ppm)
- CO: Carbon Monoxide (ppm)

## Sensor Libraries

### Compatible Libraries
- MQ Gas Sensor Library v1.3 (for MQ-2, MQ-3, MQ-4, MQ-5, MQ-6, MQ-7, MQ-8, MQ-9)
- Adafruit SGP30 Library (for CO2 and VOC sensing)
- DFRobot SO2/H2S Electrochemical Sensor Library
- Sensirion SCD30 Library (for CO2)

### Incompatible Libraries
- Seeed MiCS Gas Sensor Library

## Sample Data

### Legacy System (Old Code)

#### No Power State
```
co2 | so2 | h2s | ch4 | no2 | c2h5oh | h2 | nh3 | co |
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
400.00 | nan | nan | nan | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
```

#### With Power State
```
co2 | so2 | h2s | ch4 | no2 | c2h5oh | h2 | nh3 | co |
169.00 | 3.60 | 0.36 | 16.15 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.33 | 0.33 | 15.98 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.34 | 0.34 | 15.76 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.48 | 0.34 | 15.82 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.60 | 0.36 | 15.89 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.54 | 0.35 | 16.02 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.44 | 0.33 | 15.76 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.39 | 0.35 | 15.76 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.61 | 0.38 | 16.04 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 3.70 | 0.34 | 15.78 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 4.23 | 0.42 | 16.56 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 4.54 | 0.45 | 17.31 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 4.74 | 0.47 | 17.24 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
0.00 | 4.79 | 0.46 | 16.77 | -1.00 | -1.00 | -1.00 | -1.00 | -1.00
```

### Current System (New Code)

#### No Power State
```
co2 | so2 | h2s | ch4 | no2 | c2h5oh | h2 | nh3 | co |
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
0.00 | nan | nan | nan | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
```

#### With Power State
```
co2 | so2 | h2s | ch4 | no2 | c2h5oh | h2 | nh3 | co |
1861.00 | 4.02 | 0.39 | 16.23 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1860.75 | 3.89 | 0.40 | 16.27 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1860.50 | 3.97 | 0.40 | 16.23 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1860.28 | 3.97 | 0.39 | 16.27 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1860.05 | 3.97 | 0.39 | 16.27 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1859.84 | 3.97 | 0.39 | 16.54 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1859.66 | 3.97 | 0.40 | 16.23 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1859.52 | 4.04 | 0.40 | 16.45 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1859.43 | 4.00 | 0.39 | 16.27 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1859.30 | 3.97 | 0.39 | 16.23 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1859.16 | 3.97 | 0.39 | 16.23 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1859.06 | 4.04 | 0.40 | 16.31 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1858.98 | 3.97 | 0.40 | 16.23 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
1858.89 | 3.91 | 0.39 | 16.23 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00
```

## Notes

- "nan" indicates no data available for that parameter
- "-1.00" in old code indicates sensor not active/configured
- "0.00" in new code indicates sensor reading zero or not active
- Measurement units are in parts per million (ppm)