///----- My first watchface (time & temp) -----///

// --- libraries and definitions --- //
#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

// --- elements of watchface --- //
static Window *main_window;
static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *battery_layer;
static TextLayer *weather;
static GFont time_font;
static GFont weather_font;

// --- battery --- //
static int battery_level;
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  battery_level = state.charge_percent;
  // Print level to screen
  static char battery_buffer[8];
  snprintf(battery_buffer, sizeof(battery_buffer), "%d %%", battery_level);
  text_layer_set_text(battery_layer, battery_buffer);
}


// --- app message --- //
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d F", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(weather, weather_layer_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// --- time handler and updates --- //
static void update_time() {
  // get time structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // write time into buffer
  static char buf[8];
  strftime(buf, sizeof(buf), "%I:%M", tick_time);
  
  // display time on textlayer
  text_layer_set_text(time_layer, buf);
  
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);

  // Show the date
  text_layer_set_text(date_layer, date_buffer);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message
    app_message_outbox_send();
  }
}

// --- main window load/unload --- //
static void main_load(Window *w) {
  // get window info
  Layer *window_layer = window_get_root_layer(w);
  GRect bounds = layer_get_bounds(window_layer);
  
  // create text layer
  time_layer = text_layer_create(
      GRect(0, 65, bounds.size.w, 50));
  
  // create weather layer
  weather = text_layer_create(
    GRect(0, 130, bounds.size.w, 25));
  
  // style text layer
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorBlueMoon);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  // style weather layer
  text_layer_set_background_color(weather, GColorBlack);
  text_layer_set_text_color(weather, GColorBlueMoon);
  text_layer_set_text_alignment(weather, GTextAlignmentCenter);
  text_layer_set_text(weather, "QUE PASA?");
  
  // create text layer font
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EXTROS_50));
  text_layer_set_font(time_layer, time_font);
  
  // create weather layer font
  text_layer_set_font(weather, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  
  // add children
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(weather));
  
  // Date Layer
  date_layer = text_layer_create(GRect(0, 35, bounds.size.w, 25));
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(date_layer));
  
  // Battery Layer
  battery_layer = text_layer_create(GRect(0, 5, bounds.size.w, 25));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(battery_layer));
  text_layer_set_text(battery_layer, "battery level");
}

static void main_unload(Window *w) {
  // destroy layers
  text_layer_destroy(time_layer);
  text_layer_destroy(weather);
  //text_layer_destory(battery_layer);
  //text_layer_destory(date_layer);
  
  // destory fonts
  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(weather_font);
}

// --- init, deinit, and main --- //
static void init() {
  // create new window
  main_window = window_create();
  
  // update parameters - black background
  window_set_background_color(main_window, GColorBlack);
  
  // set handlers for window elements
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_load,
    .unload = main_unload
  });
  
  // show window, make it animated
  window_stack_push(main_window, true);
  
  // display time
  update_time();
  
  // register with timer service
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // app message register calls
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
}

static void deinit() {
  // destroy main window
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}