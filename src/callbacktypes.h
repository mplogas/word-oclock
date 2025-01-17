#ifndef CALLBACKTYPES_H
#define CALLBACKTYPES_H

// enum LightOperationType {
//   ToggleStatus,
//   SetColor,
//   SetAutoBrightness,
//   SetBrightness
// };

// enum SystemOperationType {
//   SetHaIntegration,
//   SetClockFormat,
//   ResetConfig
// };

// enum TimeOperationType {
//   SetTime,
//   SetNTPSync,
//   SetLightSchedule
// };

enum UpdateType {
    FIRMWARE,
    FILESYSTEM
};

enum ControlType {
  LightStatus,
  Color,
  Brightness,
  AutoBrightness,
  HaIntegration,
  ClockFace,
  ResetConfig,
  Time,
  NTPSync,
  LightSchedule,
  WiFi
};

enum DetailsType {
  LightConfig,
  SystemConfig,
  TimeConfig
};

#endif // CALLBACKTYPES_H