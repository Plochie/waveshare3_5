#pragma once

#include <lvgl.h>

enum class app_category
{
  instruments,
  network,
  debug,
  motion,
  camera,
  media_info,
};

struct app_descriptor_t
{
  const char *name;
  const char *icon; // text/symbol placeholder
  app_category category;
  lv_obj_t *(*create)(); // returns a new screen object
};
