#include <pebble.h>
static Window *main_window;
static TextLayer *time_layer;
static GFont time_font;

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
static void main_load(Window *w) {
  // get window info
  Layer *window_layer = window_get_root_layer(w);
  GRect bounds = layer_get_bounds(window_layer);
  
  // create textlayer
  time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58,52), bounds.size.w, 50));
  
  // make layout look like watchface
  text_layer_set_background_color(time_layer, GColorBlack);
  text_layer_set_text_color(time_layer, GColorBlueMoon);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  // create font
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EXTROS_48));
  text_layer_set_font(time_layer, time_font);
  
  // add it as child to main window layer
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
}
static void main_unload(Window *w) {
  // destroy textlayer
  text_layer_destroy(time_layer);
  
  // destory font
  fonts_unload_custom_font(time_font);
}
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