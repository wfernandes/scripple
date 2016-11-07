#include "pebble.h"

#define NUM_MENU_ITEMS 3
#define DATA_STORE_SIZE 6

static Window *s_main_window;
static Window *s_window;
static MenuLayer *s_menu_layer;
static TextLayer *details_layer;
static int num_data_store;

typedef struct {
  char str[256];
} Data;
static Data DataStore[DATA_STORE_SIZE];

static void add_data_store(int index){
  // Persist to datastore if within the store limit
  if(index >= DATA_STORE_SIZE){
    return;
  }
  snprintf(DataStore[index].str, 256, "Item %ds content goes here", index);
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return num_data_store+1;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Add the + sign menu item
  if (cell_index->row == 0) {
    menu_cell_title_draw(ctx, cell_layer, "+");
    return;
  }
  
  // Add row content with actual data from data source
  menu_cell_basic_draw(ctx, cell_layer, DataStore[cell_index->row - 1].str, NULL, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  details_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(80,80), bounds.size.w, bounds.size.h));
  text_layer_set_text(details_layer, window_get_user_data(window));
  layer_add_child(window_layer, text_layer_get_layer(details_layer));
}

static void window_unload(Window *window){
  text_layer_destroy(details_layer);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  
  if(cell_index->row == 0){
    // Create a new row with data
    num_data_store++;
    add_data_store(num_data_store-1);

    // Redraw the layer
    menu_layer_reload_data(menu_layer);
    
    return;
  }
  
  s_window = window_create();
  window_set_user_data(s_window, DataStore[cell_index->row - 1].str);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void menu_long_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->row == 0){
    // Don't delete the add row item
    return;
  }
  printf("Delete row item: %d", cell_index->row);
  for(int i = cell_index->row - 1; i < num_data_store - 1; i++){
    DataStore[i] = DataStore[i+1];
  }
  num_data_store--;
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

  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
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

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_menu_layer);
}

static void init() {
  // Lets create some data to populate the store
  num_data_store = 3;
  for (int i=0; i<num_data_store; i++){
    add_data_store(i);
  }
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
