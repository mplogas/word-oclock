#ifndef CALLBACKTYPES_H
#define CALLBACKTYPES_H

enum LightOperationType {
  ToggleStatus,
  SetColor,
  SetAutoBrightness,
  SetBrightness
};

enum SystemOperationType {
  SetHaIntegration,
  SetNTPTime,
  SetNtpAutoUpdate,
  SetLightSchedule,
  SetClockFormat,
  ResetConfig
};

#endif // CALLBACKTYPES_H