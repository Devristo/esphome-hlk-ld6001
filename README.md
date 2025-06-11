# ESPHome Components for HiLink HLK-LD6001 and HLK-LD6001A

This repository provides ESPHome components for the [HiLink HLK-LD6001](https://www.hlktech.net/index.php?id=1313) and [HLK-LD6001A](https://www.hlktech.net/index.php?id=1309) 60GHz mmWave radar sensors.

While both sensors offer similar performance, they use different communication protocols. As a result, separate ESPHome components are included for each model.

| Feature               | LD6001 | LD6001A |
|-----------------------|--------|---------|
| Frequency             | 60GHz  | 60GHz   |
| Antenna Configuration | 4T3R   | 4T4R    |
| Mount Location        | Wall   | Ceiling |
| Max People Count      | 8      | 10      |
| Max People Tracking   | 8      | 10      |
| Sensitivity Settings  | 2      | 9       |
| Configurable Range    | No     | Yes     |
| Bluetooth             | No     | No      |
| UART                  | Yes    | Yes     |

## Getting Started

See [example.yaml](./example.yaml) for configuration and usage instructions.

The component has been tested on several ESP32-S3 boards.

## Development

A devcontainer configuration is provided for a Docker-based development environment.

### Running Tests

To run tests, execute:

```sh
pio test
```

Alternatively, you can run tests using the PlatformIO extension in VSCode. Note that only part of the codebase is currently covered by unit tests.