#ifndef CALLBACKTYPES_H
#define CALLBACKTYPES_H

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
  WiFiSetup
};

enum PageType {
    LIGHT,
    TIME,
    SYSTEM,
    FWUPDATE
};

enum SchedulerType {
    Timestamp,
    ScheduleStart,
    ScheduleEnd
};

// enum DetailsType {
//   LightConfig,
//   SystemConfig,
//   TimeConfig,
//   UpdateConfig
// };

#endif // CALLBACKTYPES_H