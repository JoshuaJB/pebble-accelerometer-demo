#include <pebble.h>

Window *my_window;
TextLayer *text_layer;

// Global number of accelerometer samples taken
int sample_num = 0;

// Called every 25 samples with accelerometer data
static void display_acceleration(AccelData * data, uint32_t num_samples) {
  sample_num += num_samples;
  // Compute local average
  int x_avg = 0;
  int y_avg = 0;
  int z_avg = 0;
  for (unsigned int i = 0; i < num_samples; i++) {
    x_avg += data[i].x;
    y_avg += data[i].y;
    z_avg += data[i].z;
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "x_avg: %d, num_samples: %d", x_avg, (int)num_samples);
  x_avg /= (int)num_samples;
  y_avg /= 25;
  z_avg /= 25;
  // Display local average
  static char display_string[57]; // 10 places for each avg
  snprintf(display_string, 57, "Local Average:\nX:%d,\nY:%d,\nZ:%d\nS:%u", (int)x_avg, (int)y_avg, (int)z_avg, (unsigned int)num_samples);
  text_layer_set_text(text_layer, display_string);
}

static void main_window_load(Window *window) {
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