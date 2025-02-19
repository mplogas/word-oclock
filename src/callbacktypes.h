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

enum MQTTEvent {
    Connected,
    Disconnected,
    BrightnessCommand,
    RGBCommand,
    StateCommand,
    AutoBrightnessSwitchCommand,
    Option1SwitchCommand,
    Option2SwitchCommand,
    Option3SwitchCommand,
    Option4SwitchCommand
};

#endif // CALLBACKTYPES_H