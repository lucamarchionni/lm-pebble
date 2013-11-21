#include <pebble.h>

static const uint32_t ACC_LOG_TAG =  0x5;

static const int RESOURCE_IDS[3] = {
  RESOURCE_ID_IMAGE_ACTION_ICON_SEALION,
  RESOURCE_ID_IMAGE_ACTION_ICON_PELICAN,
  RESOURCE_ID_IMAGE_ACTION_ICON_DOLPHIN
};

static const GRect NUMBER_POSITIONS[3] =  {
  {{/* x: */ 100, /* y: */ 12 }, {/* width: */ 28, /* height: */ 28}},
  {{/* x: */ 100, /* y: */ 60 }, {/* width: */ 28, /* height: */ 28}},
  {{/* x: */ 100, /* y: */ 107 }, {/* width: */ 28, /* height: */ 28}}
};


static Window *window;
static ActionBarLayer *action_bar_layer;
static TextLayer *intro_layer;
static TextLayer * text_layer[3];
static GBitmap *bitmap[3];
static char text[3][20];
static char counter_text[30];
static DataLoggingSessionRef logging_session;
static AppTimer *timer;
static bool acceleration_log_active;
static AccelData acc_data;
static int counter;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {

  if(!acceleration_log_active)
  {
	counter = 0;
    acceleration_log_active = true;
    logging_session = data_logging_create(ACC_LOG_TAG, DATA_LOGGING_INT, 2, false);
  }
  else
  {
    acceleration_log_active = false;
    data_logging_finish(logging_session);
  }
  snprintf(text[1], 20, "%d", acceleration_log_active);
  text_layer_set_text(text_layer[1], text[1]);
  text_layer_set_text(intro_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(intro_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(intro_layer, "Down");
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  data_logging_finish(logging_session);
  logging_session = data_logging_create(ACC_LOG_TAG, DATA_LOGGING_INT, 2, false);
  text_layer_set_text(intro_layer, "Long select");
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
}

static void init_datas(Window *window) {
  counter = 0;
  for (int i = 0; i < 3; i++) {
    text_layer[i] = text_layer_create(NUMBER_POSITIONS[i]);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer[i]));
    bitmap[i] = gbitmap_create_with_resource(RESOURCE_IDS[i]);
    snprintf(text[i], 20, "%d", 0);
    text_layer_set_text(text_layer[i], text[i]);
    text_layer_set_font(text_layer[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  }
}

static void action_bar_init(Window *window) {
  action_bar_layer = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar_layer, window);
  action_bar_layer_set_click_config_provider(action_bar_layer, config_provider);
  action_bar_layer_set_icon(action_bar_layer, BUTTON_ID_UP, bitmap[0]);
  action_bar_layer_set_icon(action_bar_layer, BUTTON_ID_SELECT, bitmap[1]);
  action_bar_layer_set_icon(action_bar_layer, BUTTON_ID_DOWN, bitmap[2]);
}

static void deinit_datas(void) {
  for (int i = 0; i < 3; i++) {
    data_logging_finish(logging_session);
    text_layer_destroy(text_layer[i]);
    gbitmap_destroy(bitmap[i]);
  }
}

static void window_load(Window *window) {
 init_datas(window);
  action_bar_init(window);
  intro_layer = text_layer_create(GRect(7, 50, 90, 93));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(intro_layer));
  text_layer_set_text(intro_layer, "Press to log\n * \n accelerometer");
  text_layer_set_text_alignment(intro_layer, GTextAlignmentCenter);
}

static void window_unload(Window *window) {
  	deinit_datas();
	action_bar_layer_destroy(action_bar_layer);
	text_layer_destroy(intro_layer);
}

static void timer_callback(void *data) {
  if(acceleration_log_active)
  {
	counter++;
	accel_service_peek(&acc_data);
	DataLoggingResult result = data_logging_log(logging_session, &(acc_data.x), 1);
	if(result == DATA_LOGGING_BUSY)
	{
	  text_layer_set_text(intro_layer, "Log busy");
	}
	else if (result == DATA_LOGGING_FULL)
	{
	  text_layer_set_text(intro_layer, "Log full");
	}
	else if (result == DATA_LOGGING_NOT_FOUND)
	{
	  text_layer_set_text(intro_layer, "Log not found");
	}
	else if (result == DATA_LOGGING_CLOSED)
	{
	  text_layer_set_text(intro_layer, "Log closed");
	}
	else if (result == DATA_LOGGING_INVALID_PARAMS)
	{
	  text_layer_set_text(intro_layer, "Log invalid params");
	}
	else
	{
	  snprintf(counter_text, 30, "Log success %d", counter);
      text_layer_set_text(intro_layer, counter_text);
    }
  }
  timer = app_timer_register(100 /* milliseconds */, timer_callback, NULL);
}

static void handle_accel(AccelData *accel_data, uint32_t num_samples) {
  // do nothing
}

static void init(void) {
  acceleration_log_active = false;
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  accel_data_service_subscribe(0, NULL);
  timer = app_timer_register(100 /* milliseconds */, timer_callback, NULL);
}

static void deinit(void) {
	accel_data_service_unsubscribe();
    window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}
