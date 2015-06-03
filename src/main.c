#include <pebble.h>

Window *my_window;
TextLayer *text_layer;

// isqrt_impl and isqrt provided by Siu Ching Pong on stackoverflow
uint32_t isqrt_impl(uint64_t const n, uint64_t const xk) {
    uint64_t const xk1 = (xk + n / xk) / 2;
    return (xk1 >= xk) ? xk : isqrt_impl(n, xk1);
}
uint32_t isqrt(uint64_t const n) {
    if (n == 0) return 0;
    if (n == 18446744073709551615ULL) return 4294967295U;
    return isqrt_impl(n, n);
}

// Called with accelerometer data samples
static void display_acceleration(AccelData * data, uint32_t num_samples) {
  // Compute local average
  int local_x_avg = 0;
  int local_y_avg = 0;
  int local_z_avg = 0;
  for (unsigned int i = 0; i < num_samples; i++) {
    local_x_avg += data[i].x;
    local_y_avg += data[i].y;
    local_z_avg += data[i].z;
  }
  local_x_avg /= (int)num_samples;
  local_y_avg /= 25;
  local_z_avg /= 25;
  // Compute the vector magnitude, subtract 1.04G for earth's gravity, and possibly a bit more for calibration
  int variance = isqrt(local_x_avg*local_x_avg + local_y_avg*local_y_avg + local_z_avg*local_z_avg) - 1040;
  // Display local average
  static char display_string[85];
  snprintf(display_string, 85, "%d Sample Average:\nX:%d,\nY:%d,\nZ:%d\nVariance From Zero\n%d", (int)num_samples, local_x_avg, local_y_avg, local_z_avg, variance);
  text_layer_set_text(text_layer, display_string);
}

static void main_window_load(Window *window) {
  // Setup the text layer
  text_layer = text_layer_create(layer_get_bounds(window_get_root_layer(my_window)));
  text_layer_set_text(text_layer, "Acquiring data...");
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(text_layer));
  // Set Accelerometer to 100Hz sample rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_25HZ);
  // Register event handler with 25 sample buffer
  accel_data_service_subscribe(25, display_acceleration);
}

void handle_init(void) {
  my_window = window_create();
  // Setup main_window_load to run once the window loads
  window_set_window_handlers(my_window, (WindowHandlers){.load = main_window_load});
  window_stack_push(my_window, true);
}

void handle_deinit(void) {
  // De-register event handler
  accel_data_service_unsubscribe();
  text_layer_destroy(text_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}