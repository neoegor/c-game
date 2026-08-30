#include "common.h"
#include "ui/notifier.h"

void notifier_init(Notifier* notifier) {
    notifier->message = NULL;
    notifier->type = NOTIFICATION_NONE;
    notifier->time_remaining = 0.0f;
}

void notifier_show(
    Notifier* notifier,
    NotificationType type,
    const char *message
) {
    notifier->message = message;
    notifier->type = type;
    notifier->time_remaining = NOTIFICATION_DURATION;
}

void notifier_update(Notifier* notifier, float dt) {
    if (notifier->time_remaining <= 0.0f) return;

    notifier->time_remaining -= dt;

    if (notifier->time_remaining <= 0.0f) {
        notifier->message = NULL;
        notifier->type = NOTIFICATION_NONE;
        notifier->time_remaining = 0.0f;
    }
}
