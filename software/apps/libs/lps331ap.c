#include "tock.h"
#include "lps331ap.h"

struct lps331ap_data {
  bool fired;
  int value;
};

static struct lps331ap_data result = { .fired = false };

// Internal callback for faking synchronous reads
static void lps331ap_cb(__attribute__ ((unused)) int value,
                       __attribute__ ((unused)) int unused1,
                       __attribute__ ((unused)) int unused2,
                       void* ud) {
  struct lps331ap_data* result = (struct lps331ap_data*) ud;
  result->value = value;
  result->fired = true;
}

int lps331ap_set_callback (subscribe_cb callback, void* callback_args) {
    return subscribe(DRIVER_NUM_LPS331AP, 0, callback, callback_args);
}

int lps331ap_get_pressure () {
    return command(DRIVER_NUM_LPS331AP, 0, 0);
}

int lps331ap_get_pressure_sync () {
    int err;
    result.fired = false;

    err = lps331ap_set_callback(lps331ap_cb, (void*) &result);
    if (err < 0) return err;

    err = lps331ap_get_pressure();
    if (err < 0) return err;

    // Wait for the callback.
    yield_for(&result.fired);

    return result.value;
}
