#include "pebble.h"

#define MAX_NUM_ITEMS 10
#define ITEM_SIZE 256
#define DATA_STORE_KEY 100

static Window *s_main_window;
static Window *s_details_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_details_layer;
static StatusBarLayer *s_status_layer;
static DictationSession *s_dictation_session;
static char s_dictated_text[ITEM_SIZE];

typedef struct {
  char str[ITEM_SIZE];
} data_t;

typedef struct {
  int num_items;
  data_t items[MAX_NUM_ITEMS];
} data_store_t;

data_store_t scripples;

static void add_dictated_data_store() {
  // Do not persist to datastore if greater than store limit
  if(scripples.num_items + 1 > MAX_NUM_ITEMS){
    return;
  }
  ++scripples.num_items;
  snprintf(scripples.items[scripples.num_items - 1].str, ITEM_SIZE, "%s", s_dictated_text);
  
  // Redraw the layer
  menu_layer_reload_data(s_menu_layer);
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
                                     char *transcription, void *context) {
  if(status != DictationSessionStatusSuccess) {
    printf("Dictation Error: %d\n", (int)status);
    return;    
  }
  snprintf(s_dictated_text, sizeof(s_dictated_text), "%s", transcription);
  add_dictated_data_store();
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return scripples.num_items+1;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->row == 0) {
    // Render the '+' in the center of the cell layer
    graphics_draw_text(ctx, "+", fonts_get_system_font(FONT_KEY_GOTHIC_28),
                      layer_get_frame(cell_layer), GTextOverflowModeWordWrap,
                      GTextAlignmentCenter, NULL);
    return;
  }
  
  // Add row content with actual data from data source
  menu_cell_basic_draw(ctx, cell_layer, scripples.items[cell_index->row - 1].str, "", NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  s_details_layer = text_layer_create(bounds);
  text_layer_set_font(s_details_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text(s_details_layer, window_get_user_data(window));
  layer_add_child(window_layer, text_layer_get_layer(s_details_layer));
}

static void window_unload(Window *window){
  text_layer_destroy(s_details_layer);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if(cell_index->row == 0){
    // Start the dictation session
    dictation_session_start(s_dictation_session);
    return;
  }
  
  s_details_window = window_create();
  window_set_user_data(s_details_window, scripples.items[cell_index->row - 1].str);
  window_set_window_handlers(s_details_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_details_window, true);
}

static void menu_long_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->row == 0){
    // Don't delete the add row item
    return;
  }
  for(int i = cell_index->row - 1; i < scripples.num_items - 1; i++){
    scripples.items[i] = scripples.items[i+1];
  }
  scripples.num_items--;
  // Redraw the menu layer
  menu_layer_reload_data(menu_layer);
}


#ifdef PBL_ROUND 
static int16_t get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) { 
  if (menu_layer_is_index_selected(menu_layer, cell_index)) {
    switch (cell_index->row) {
      case 0:
        return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
        break;
      default:
        return MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT;
    }
  } else {
    return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
  }
}
#endif

static void main_window_load(Window *window) {
  s_status_layer = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_layer, GColorChromeYellow, GColorBlack);
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  // To make room for the status bar
  bounds.origin.y = STATUS_BAR_LAYER_HEIGHT;

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = NULL,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = NULL,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .select_long_click = menu_long_select_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(get_cell_height_callback, NULL),
  });

  menu_layer_set_highlight_colors(s_menu_layer, GColorSunsetOrange, GColorBlack);
  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_layer));
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  
  menu_layer_reload_data(s_menu_layer);
}

static void main_window_unload(Window *window) {
  status_bar_layer_destroy(s_status_layer);
  menu_layer_destroy(s_menu_layer);
}

static void init() {
  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_dictated_text), dictation_session_callback, NULL);
  
  // Load persisted data if any
  if (persist_exists(DATA_STORE_KEY)) {
    scripples.num_items = persist_read_int(DATA_STORE_KEY);
    
    for(int i=1; i<=scripples.num_items; i++){
      persist_read_string(DATA_STORE_KEY+i, &scripples.items[i-1].str[0], sizeof(data_t));
    }
  } 
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  persist_write_int(DATA_STORE_KEY, scripples.num_items);
  for(int i=1; i<=scripples.num_items; i++){
    persist_write_string(DATA_STORE_KEY+i, &scripples.items[i-1].str[0]);
  }
  dictation_session_destroy(s_dictation_session);
  window_destroy(s_main_window);
  window_destroy(s_details_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
