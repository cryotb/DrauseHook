#pragma once

namespace core
{
     void on_render(int mode);
     LLVM_NOOPT void on_level_init(const char* map);
     LLVM_NOOPT void on_level_shutdown();
     void erase_name(Entity &ent);
     bool is_firing_range();
}
