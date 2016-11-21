#include <pebble.h>
#include "status_bar.h"

static StatusBarLayer *s_status_layer;

void status_bar_load(Layer *root_layer) {
  s_status_layer = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_layer, GColorChromeYellow, GColorBlack);
  
  layer_add_child(root_layer, status_bar_layer_get_layer(s_status_layer));
}

GRect status_bar_bounds(Layer *root_layer) {
  GRect bounds = layer_get_bounds(root_layer);
  // To make room for the status bar
  bounds.origin.y = STATUS_BAR_LAYER_HEIGHT;
  return bounds;
}

void status_bar_unload(){
  status_bar_layer_destroy(s_status_layer);
}