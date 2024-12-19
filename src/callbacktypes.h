#ifndef CALLBACKTYPES_H
#define CALLBACKTYPES_H

enum class LightOperationType {
  ToggleStatus,
  SetColor,
  SetAutoBrightness,
  SetBrightness
};

enum class SystemOperationType {
  SetNTPServer,
  SetTimezone,
  SetMQTTConfig,
  SetMQTTUser
};

#endif // CALLBACKTYPES_H