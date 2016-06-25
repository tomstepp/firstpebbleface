#include <pebble.h>

// --- elements of watchface --- //

static Window *main_window;

static TextLayer *time_layer;
static TextLayer *weather;

static GFont time_font;
static GFont weather_font;

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
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

// --- main window load/unload --- //

static void main_load(Window *w) {
  // get window info
  Layer *window_layer = window_get_root_layer(w);
  GRect bounds = layer_get_bounds(window_layer);
  
  // create text layer
  time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58,52), bounds.size.w, 50));
  
  // create weather layer
  weather = text_layer_create(GRect(0, PBL_IF_BW_ELSE(125, 120), bounds.size.w, 25));
  
  // style text layer
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorBlueMoon);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  // style weather layer
  text_layer_set_background_color(weather, GColorBlack);
  text_layer_set_text_color(weather, GColorBlueMoon);
  text_layer_set_text_alignment(weather, GTextAlignmentCenter);
  text_layer_set_text(weather, "LOADING...");
  
  // create text font
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EXTROS_48));
  text_layer_set_font(time_layer, time_font);
  
  // create weather font
  weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EXTROS_18));
  text_layer_set_font(weather, weather_font);
  
  // add children
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(weather));
}

static void main_unload(Window *w) {
  // destroy layers
  text_layer_destroy(time_layer);
  text_layer_destroy(weather);
  
  // destory fonts
  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(weather_font);
}

// --- app message --- //

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
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
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
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