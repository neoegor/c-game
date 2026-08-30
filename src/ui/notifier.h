#ifndef NOTIFIER_H
#define NOTIFIER_H

#include "common.h"

#define NOTIFICATION_DURATION 1.5f
#define NOTIFICATION_FADE_DURATION 0.5f

typedef enum {
    NOTIFICATION_NONE,
    NOTIFICATION_INFO,
    NOTIFICATION_ERROR,
} NotificationType;

typedef struct {
    const char *message;
    NotificationType type;
    float time_remaining;
} Notifier;

void notifier_init(Notifier* notifier);
void notifier_show(
    Notifier* notifier,
    NotificationType type,
    const char *message
);
void notifier_update(Notifier* notifier, float dt);

#endif
