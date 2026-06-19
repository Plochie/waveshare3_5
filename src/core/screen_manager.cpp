#include "core/screen_manager.h"

#include <vector>

namespace screen_manager {

static std::vector<lv_obj_t *> stack;

void init()
{
  stack.clear();
}

void push(lv_obj_t *screen)
{
  stack.push_back(screen);
  lv_scr_load(screen);
}

void pop()
{
  if (stack.size() <= 1)
  {
    return;
  }

  lv_obj_t *top = stack.back();
  stack.pop_back();

  lv_scr_load(stack.back());
  lv_obj_delete(top);
}

} // namespace screen_manager
