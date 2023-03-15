// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "pugl/gl.h"
#if !__APPLE__
#include "GL/glext.h"
#endif
#include "smwindow.hh"
#include "smmenubar.hh"
#include "smscrollview.hh"
#include "smnativefiledialog.hh"
#include "smconfig.hh"
#include "smshortcut.hh"
#include "smeventloop.hh"
#include "smutils.hh"
#include "pugl/cairo_gl.h"
#include <map>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::map;
using std::min;
using std::max;

struct SpectMorph::CairoGL
{
private:
  PuglCairoGL pugl_cairo_gl;
  cairo_surface_t *surface;
  int  m_width;
  int  m_height;

public:
  cairo_t *cr;

  CairoGL (int width, int height) :
    m_width (width), m_height (height)
  {
    memset (&pugl_cairo_gl, 0, sizeof (pugl_cairo_gl));

    surface = pugl_cairo_gl_create (&pugl_cairo_gl, width, height, 4);
    cr = cairo_create (surface);
  }
  ~CairoGL()
  {
    cairo_destroy (cr);
    cairo_surface_destroy (surface);

    pugl_cairo_gl_free (&pugl_cairo_gl);
  }
  void
  configure()
  {
    pugl_cairo_gl_configure (&pugl_cairo_gl, m_width, m_height);

    glPixelStorei (GL_UNPACK_ROW_LENGTH, m_width);
    glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8,
                  m_width, m_height, 0,
                  GL_BGRA, GL_UNSIGNED_BYTE, pugl_cairo_gl.buffer);
  }
  void
  draw (int x, int y, int w, int h)
  {
    (void) pugl_cairo_gl_draw; // reimplement this:

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glEnable(GL_TEXTURE_2D);

    // update modified part of the texture from cairo buffer
    uint32 *src_buffer = reinterpret_cast<uint32 *> (pugl_cairo_gl.buffer);

    glTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0,
                     x, y, w, h,
                     GL_BGRA, GL_UNSIGNED_BYTE, src_buffer + y * m_width + x);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, (GLfloat)m_height);
    glVertex2f(-1.0f, -1.0f);

    glTexCoord2f((GLfloat)m_width, (GLfloat)m_height);
    glVertex2f(1.0f, -1.0f);

    glTexCoord2f((GLfloat)m_width, 0.0f);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    glPopMatrix();
  }
  int
  width()
  {
    return m_width;
  }
  int
  height()
  {
    return m_height;
  }
  uint32 *
  buffer()
  {
    return reinterpret_cast<uint32 *> (pugl_cairo_gl.buffer);
  }
};

static void
on_event (PuglView* view, const PuglEvent* event)
{
  Window *window = reinterpret_cast<Window *> (puglGetHandle (view));

  if (window->event_loop()->window_alive (window))
    window->on_event (event);
}

static void
on_resize (PuglView *view, int *width, int *height, int *set_hints)
{
  Window *window = reinterpret_cast<Window *> (puglGetHandle (view));

  if (window->event_loop()->window_alive (window))
    window->on_resize (width, height);
}

Window::Window (EventLoop& event_loop, const string& title, int width, int height, PuglNativeWindow win_id, bool resize, PuglNativeWindow transient_parent) :
  Widget (nullptr, 0, 0, width, height),
  draw_grid (false)
{
  Config cfg;

  global_scale = cfg.zoom() / 100.0;

  view = puglInit (nullptr, nullptr);

  /* draw 128 bits from random generator to ensure that window class name is unique */
  string window_class = "SpectMorph_";
  for (size_t i = 0; i < 4; i++)
    window_class += string_printf ("%08x", g_random_int());

  int scaled_width, scaled_height;
  get_scaled_size (&scaled_width, &scaled_height);

  puglInitWindowClass (view, window_class.c_str());
  puglInitWindowSize (view, scaled_width, scaled_height);
  //puglInitWindowMinSize (view, 256, 256);
  puglInitResizable (view, resize);
  puglIgnoreKeyRepeat (view, true);
  if (win_id)
    puglInitWindowParent (view, win_id);
  if (transient_parent)
    puglInitTransientFor (view, transient_parent);
  puglCreateWindow (view, title.c_str());

  puglSetHandle (view, this);
  puglSetEventFunc (view, ::on_event);
  puglSetResizeFunc (view, ::on_resize);

  cairo_gl.reset (new CairoGL (scaled_width, scaled_height));
  update_full();

  puglEnterContext (view);
  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  glClearColor (0.4f, 0.4f, 0.4f, 1.0f);
  cairo_gl->configure();
  // printf ("OpenGL Version: %s\n",(const char*) glGetString(GL_VERSION));
  puglLeaveContext(view, false);

  set_background_color (ThemeColor::WINDOW_BG);

  m_event_loop = &event_loop;
  m_event_loop->add_window (this);
}

Window::~Window()
{
  m_event_loop->remove_window (this);
  puglDestroy (view);

  /* cleanup shortcuts: this code needs to work if remove_shortcut & add_shortcut are called from one of the destructors */
  for (size_t i = 0; i < shortcuts.size(); i++)
    {
      if (shortcuts[i])
        delete shortcuts[i];
    }
  for (size_t i = 0; i < shortcuts.size(); i++)
    assert (shortcuts[i] == nullptr);
}

void
Window::on_widget_deleted (Widget *child)
{
  /* cheap weak pointer emulation */
  if (mouse_widget == child)
    mouse_widget = nullptr;
  if (enter_widget == child)
    enter_widget = nullptr;
  if (menu_widget == child)
    menu_widget = nullptr;
  if (keyboard_focus_widget == child)
    keyboard_focus_widget = nullptr;
  if (dialog_widget == child)
    {
      update_full();
      dialog_widget = nullptr;
    }

  event_loop()->on_widget_deleted (child);
}

static vector<Widget *>
crawl_widgets (const vector<Widget *>& widgets)
{
  vector<Widget *> all_widgets;

  for (auto w : widgets)
    {
      all_widgets.push_back (w);
      auto c_result = crawl_widgets (w->children);
      all_widgets.insert (all_widgets.end(), c_result.begin(), c_result.end());
    }
  return all_widgets;
}

vector<Widget *>
Window::crawl_widgets()
{
  return ::crawl_widgets ({ this });
}

static int
get_layer (Widget *w, Widget *menu_widget, Widget *dialog_widget)
{
  if (w == menu_widget)
    return 1;
  if (w == dialog_widget)
    return 2;

  if (w->parent)
    return get_layer (w->parent, menu_widget, dialog_widget);
  else
    return 0; // no parent
}

static bool
get_visible_recursive (Widget *w)
{
  if (!w->visible())
    return false;

  if (w->parent)
    return get_visible_recursive (w->parent);
  else
    return true; // no parent
}

namespace {

struct IRect
{
  int x, y, w, h;

  IRect (const Rect& r, double scale)
  {
    x = r.x() * scale;
    y = r.y() * scale;
    w = r.width() * scale;
    h = r.height() * scale;
  }

  void
  grow (int px)
  {
    x -= px;
    y -= px;
    w += px * 2;
    h += px * 2;
  }

  void
  clip (int width, int height)
  {
    x = sm_bound (0, x, width);
    y = sm_bound (0, y, height);
    w = sm_bound (0, w, width - x);
    h = sm_bound (0, h, height - y);
  }
};

};

void
Window::collect_widgets_for_redraw (RedrawParams& redraw_params, Widget *widget, int layer)
{
  if (widget->visible())
    {
      if (widget == menu_widget)
        layer = 1;
      if (widget == dialog_widget)
        layer = 2;

      redraw_params.visible_widgets_by_layer[layer].push_back (widget);
      for (Widget *child : widget->children)
        collect_widgets_for_redraw (redraw_params, child, layer);
    }
}

void
Window::on_expose_event (const PuglEventExpose& event)
{
  RedrawParams redraw_params;
  redraw_params.visible_widgets_by_layer.resize (3);
  collect_widgets_for_redraw (redraw_params, this, 0);

  if (update_full_redraw)
    {
      redraw_params.full_redraw = true;
      redraw_update_region (redraw_params);
    }
  else
    {
      for (const auto& update_region : update_regions)
        {
          Widget *widget = update_region.update_strategy == UPDATE_LOCAL ? update_region.widget : nullptr;

          Rect& rect = redraw_params.merged_regions[widget];
          rect = rect.rect_union (update_region.region);
        }

      if (debug_update_region)
        {
          redraw_update_region (redraw_params);
        }
      else
        {
          for (const auto& [widget, rect] : redraw_params.merged_regions)
            {
              redraw_params.update_region = rect;
              redraw_update_region (redraw_params);
            }
        }
    }
  update_full_redraw = false;
  update_regions.clear();
}

void
Window::redraw_update_region (const RedrawParams& redraw_params)
{
  bool full_redraw = redraw_params.full_redraw;

  cairo_save (cairo_gl->cr);

  /* for debugging, we want a rectangle around the area we would normally update */
  const bool draw_update_region_rect = debug_update_region && !update_full_redraw;
  if (draw_update_region_rect)
    full_redraw = true;   // always draw full frames in debug mode

  Rect update_region_larger;
  if (!full_redraw)
    {
      // setup clipping - only need to redraw part of the screen
      IRect r (redraw_params.update_region, global_scale);

      // since we have to convert double coordinates to integer, we add a bit of extra space
      r.grow (4);

      cairo_rectangle (cairo_gl->cr, r.x, r.y, r.w, r.h);
      cairo_clip (cairo_gl->cr);

      update_region_larger = Rect (r.x / global_scale, r.y / global_scale, r.w / global_scale, r.h / global_scale);
    }

  for (int layer = 0; layer < 3; layer++)
    {
      if (dialog_widget && layer == 2) /* draw rest of ui darker if dialog is open */
        {
          cairo_rectangle (cairo_gl->cr, 0, 0, width() * global_scale, height() * global_scale);
          cairo_set_source_rgba (cairo_gl->cr, 0.0, 0, 0, 0.5);
          cairo_fill (cairo_gl->cr);
        }
      for (auto w : redraw_params.visible_widgets_by_layer[layer])
        {
          Rect visible_rect = w->abs_visible_rect();
          if (!full_redraw)
            {
              // only redraw changed parts
              visible_rect = visible_rect.intersection (update_region_larger);
            }
          if (!visible_rect.empty() || !w->clipping())
            {
              cairo_t *cr = cairo_gl->cr;

              cairo_save (cr);
              cairo_scale (cr, global_scale, global_scale);

              DrawEvent devent;

              // local coordinates
              cairo_translate (cr, w->abs_x(), w->abs_y());
              if (w->clipping())
                {
                  // translate to widget local coordinates
                  visible_rect.move_to (visible_rect.x() - w->abs_x(), visible_rect.y() - w->abs_y());

                  cairo_rectangle (cr, visible_rect.x(), visible_rect.y(), visible_rect.width(), visible_rect.height());
                  cairo_clip (cr);

                  devent.rect = visible_rect;
                }

              if (draw_grid && w == enter_widget)
                w->debug_fill (cr);

              devent.cr = cr;
              w->draw (devent);
              cairo_restore (cr);
            }
        }
    }

  if (have_file_dialog || popup_window)
    {
      cairo_rectangle (cairo_gl->cr, 0, 0, width() * global_scale, height() * global_scale);
      cairo_set_source_rgba (cairo_gl->cr, 0.0, 0, 0, 0.5);
      cairo_fill (cairo_gl->cr);
    }

  if (draw_grid)
    {
      cairo_t *cr = cairo_gl->cr;

      cairo_save (cr);
      cairo_scale (cr, global_scale, global_scale);

      cairo_set_source_rgb (cr, 0, 0, 0);
      cairo_set_line_width (cr, 0.5);

      for (double x = 8; x < width(); x += 8)
        {
          for (double y = 8; y < height(); y += 8)
            {
              cairo_move_to (cr, x - 2, y);
              cairo_line_to (cr, x + 2, y);
              cairo_move_to (cr, x, y - 2);
              cairo_line_to (cr, x, y + 2);
            }
        }
      cairo_stroke (cr);
      cairo_restore (cr);
    }
  cairo_restore (cairo_gl->cr);

  if (draw_update_region_rect)
    {
      for (const auto& [widget, region] : redraw_params.merged_regions)
        {
          cairo_t *cr = cairo_gl->cr;

          cairo_save (cr);
          cairo_scale (cr, global_scale, global_scale);
          cairo_rectangle (cr, region.x(), region.y(), region.width(), region.height());
          cairo_set_source_rgb (cr, 1.0, 0.4, 0.4);
          cairo_set_line_width (cr, 3.0);
          cairo_stroke (cr);
          cairo_restore (cr);
        }
    }

  if (full_redraw)
    {
      cairo_gl->draw (0, 0, cairo_gl->width(), cairo_gl->height());
    }
  else
    {
      IRect draw_region (redraw_params.update_region, global_scale);

      /* the number of pixels we blit is somewhat smaller than "update_region_larger", so we have
       *
       * update_region < draw_region < update_region larger
       *
       * the extra space should compensate for the effect of fractional
       * coordinates (which do not render at exact pixel boundaries)
       */
      draw_region.grow (2);

      draw_region.clip (cairo_gl->width(), cairo_gl->height());

      cairo_gl->draw (draw_region.x, draw_region.y, draw_region.w, draw_region.h);
    }
}

void
Window::init_sprite()
{
  const int RADIUS = 6;
  int spr_width = (RADIUS * 2 + 2) * global_scale;
  int spr_height = (RADIUS * 2 + 2) * global_scale;

  if (sprite.width == spr_width && sprite.height == spr_height)
    return;

  sprite.width = spr_width;
  sprite.height = spr_height;
  sprite.data.resize (spr_width * spr_height);
  std::fill (sprite.data.begin(), sprite.data.end(), 0);

  cairo_surface_t *surface = cairo_image_surface_create_for_data (reinterpret_cast<unsigned char *> (sprite.data.data()),
                                                                  CAIRO_FORMAT_ARGB32, spr_width, spr_height, spr_width * 4);
  cairo_t *cr = cairo_create (surface);

  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  cairo_scale (cr, global_scale, global_scale);
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_arc (cr, RADIUS + 1, RADIUS + 1, RADIUS, 0, 2 * M_PI);
  cairo_fill (cr);

  cairo_destroy (cr);
  cairo_surface_destroy (surface);
}

void
Window::get_sprite_size (double& w, double& h)
{
  init_sprite();

  w = sprite.width / global_scale;
  h = sprite.height / global_scale;
}

/*
 * software sprites: drawing lots of circles using plain cairo is really slow,
 * so we prepare a circle once, so that drawing it can be done quickly
 *
 * other sprite shapes could be added as needed
 */
void
Window::draw_sprite (Widget *widget, double x, double y)
{
  init_sprite();

  auto vrect = widget->abs_visible_rect();
  int startx = max<int> (0, vrect.x() * global_scale);
  int starty = max<int> (0, vrect.y() * global_scale);
  int endx = min<int> (cairo_gl->width(), (vrect.x() + vrect.width()) * global_scale);
  int endy = min<int> (cairo_gl->height(), (vrect.y() + vrect.height()) * global_scale);

  int sx = (x + widget->abs_x()) * global_scale;
  int sy = (y + widget->abs_y()) * global_scale;
  uint32 *src_buffer = cairo_gl->buffer();

  const int spr_width = sprite.width;
  const int spr_height = sprite.height;
  const uint32* spr_data = sprite.data.data();

  for (int dx = 0; dx < spr_width; dx++)
    {
      for (int dy = 0; dy < spr_height; dy++)
        {
          if (sy + dy >= starty && sy + dy < endy)
            {
              if (sx + dx >= startx && sx + dx < endx)
                {
                  if (spr_data[dy * spr_width + dx])
                    src_buffer[sx + dx + (sy + dy) * cairo_gl->width()] = 0xffaaaaaa;
                }
            }
        }
    }
}

void
Window::process_events()
{
  assert (m_event_loop);
  assert (m_event_loop->level() == 1);

  if (native_file_dialog)
    {
      native_file_dialog->process_events();

      if (!have_file_dialog)
        {
          /* file dialog closed - must be deleted after (not during) process_events */
          native_file_dialog.reset();
        }
    }

  if (0)
    {
      static double last_time = -1;
      const double time = get_time();
      const double delta_time = time - last_time;

      if (last_time > 0)
        sm_debug ("process_delta_time %f %f\n", /* time diff */ delta_time, /* frames per second */ 1.0 / delta_time);

      last_time = time;
    }
  puglProcessEvents (view);
}

static void
dump_event (const PuglEvent *event)
{
  switch (event->type)
    {
      case PUGL_NOTHING:            printf ("Event: nothing\n");
        break;
      case PUGL_BUTTON_PRESS:       printf ("Event: button press\n");
        break;
      case PUGL_BUTTON_RELEASE:     printf ("Event: button release\n");
        break;
      case PUGL_CONFIGURE:          printf ("Event: configure w%f h%f\n", event->configure.width, event->configure.height);
        break;
      case PUGL_EXPOSE:             printf ("Event: expose x%f y%f w%f h%f\n", event->expose.x, event->expose.y, event->expose.width, event->expose.height);
        break;
      case PUGL_CLOSE:              printf ("Event: close\n");
        break;
      case PUGL_KEY_PRESS:          printf ("Event: key press %c\n", event->key.character);
        break;
      case PUGL_KEY_RELEASE:        printf ("Event: key release\n");
        break;
      case PUGL_ENTER_NOTIFY:       printf ("Event: enter\n");
        break;
      case PUGL_LEAVE_NOTIFY:       printf ("Event: leave\n");
        break;
      case PUGL_MOTION_NOTIFY:      printf ("Event: motion\n");
        break;
      case PUGL_SCROLL:             printf ("Event: scroll: dx=%f dy=%f\n", event->scroll.dx, event->scroll.dy);
        break;
      case PUGL_FOCUS_IN:           printf ("Event: focus in\n");
        break;
      case PUGL_FOCUS_OUT:          printf ("Event: focus out\n");
        break;
    }
}

void
Window::on_event (const PuglEvent* event)
{
  if (0)
    dump_event (event);

  /* as long as the file dialog or popup window is open, ignore user input */
  const bool ignore_input = have_file_dialog || popup_window;
  if (ignore_input && event->type != PUGL_EXPOSE && event->type != PUGL_CONFIGURE)
    return;

  switch (event->type)
    {
      case PUGL_BUTTON_PRESS:
      case PUGL_BUTTON_RELEASE: on_button_event (event->button);
                                break;
      case PUGL_MOTION_NOTIFY:  on_motion_event (event->motion);
                                break;
      case PUGL_SCROLL:         on_scroll_event (event->scroll);
                                break;
      case PUGL_KEY_PRESS:      on_key_event (event->key);
                                break;
      case PUGL_EXPOSE:         on_expose_event (event->expose);
                                break;
      case PUGL_CLOSE:          on_close_event (event->close);
                                break;
      case PUGL_CONFIGURE:      on_configure_event (event->configure);
                                break;
      default:                  break;
    }
}

static MouseButton
to_mouse_button (unsigned pugl_button)
{
  switch (pugl_button)
  {
    case 1: return LEFT_BUTTON;
    case 2: return MIDDLE_BUTTON;
    case 3: return RIGHT_BUTTON;
  }
  return NO_BUTTON;
}

void
Window::on_button_event (const PuglEventButton& event)
{
  const double ex = event.x / global_scale;
  const double ey = event.y / global_scale;

  if (event.type == PUGL_BUTTON_PRESS)
    {
      if (!mouse_buttons_pressed)
        mouse_widget = find_widget_xy (ex, ey);
      mouse_buttons_pressed |= to_mouse_button (event.button);

      if (keyboard_focus_widget &&
          keyboard_focus_release_on_click &&
          mouse_widget != keyboard_focus_widget) /* we only release focus if the click is outside the widget */
        {
          keyboard_focus_widget->focus_out_event();
          keyboard_focus_widget = nullptr;
        }

      MouseEvent mouse_event;
      mouse_event.x = ex - mouse_widget->abs_x();
      mouse_event.y = ey - mouse_widget->abs_y();
      mouse_event.button = to_mouse_button (event.button);
      mouse_event.buttons = mouse_buttons_pressed;
      mouse_event.state = event.state;

      Point pos (mouse_event.x, mouse_event.y);
      mouse_event.double_click = (event.time - last_click_time < 0.4 &&
                                  event.button == last_click_button &&
                                  pos.distance (last_click_pos) < 15);

      last_click_time = event.time;
      last_click_button = event.button;
      last_click_pos = pos;

      mouse_widget->mouse_press (mouse_event);
    }
  else /* event.type == PUGL_BUTTON_RELEASE */
    {
      mouse_buttons_pressed &= ~to_mouse_button (event.button);
      if (mouse_widget)
        {
          MouseEvent mouse_event;
          mouse_event.x = ex - mouse_widget->abs_x();
          mouse_event.y = ey - mouse_widget->abs_y();
          mouse_event.button = to_mouse_button (event.button);
          mouse_event.buttons = mouse_buttons_pressed;
          mouse_event.state = event.state;
          mouse_widget->mouse_release (mouse_event);

          if (!mouse_buttons_pressed) /* last mouse button released nulls mouse_widget */
            mouse_widget = nullptr;
        }
    }
}

void
Window::on_motion_event (const PuglEventMotion& event)
{
  const double ex = event.x / global_scale;
  const double ey = event.y / global_scale;
  Widget *current_widget = mouse_widget;

  if (!current_widget)
    {
      current_widget = find_widget_xy (ex, ey);
      if (enter_widget != current_widget)
        {
          if (enter_widget)
            enter_widget->leave_event();

          enter_widget = current_widget;
          current_widget->enter_event();
        }
    }
  MouseEvent mouse_event;
  mouse_event.x = ex - current_widget->abs_x();
  mouse_event.y = ey - current_widget->abs_y();
  mouse_event.buttons = mouse_buttons_pressed;
  mouse_event.state = event.state;
  current_widget->mouse_move (mouse_event);
}

void
Window::on_scroll_event (const PuglEventScroll& event)
{
  const double ex = event.x / global_scale;
  const double ey = event.y / global_scale;
  Widget *current_widget = mouse_widget;

  if (!current_widget)
    current_widget = find_widget_xy (ex, ey);

  while (current_widget)
    {
      if (current_widget->scroll (event.dx, event.dy))
        break;

      current_widget = current_widget->parent;
    }
}

void
Window::on_key_event (const PuglEventKey& event)
{
  bool key_handled = false;
  /* do not use auto here, since shortcuts may get modified */
  cleanup_null (shortcuts);
  for (size_t i = 0; i < shortcuts.size(); i++)
    {
      Shortcut *shortcut = shortcuts[i];
      if (!key_handled && shortcut)
        {
          if (!keyboard_focus_widget || !shortcut->focus_override())
            key_handled = shortcut->key_press_event (event);
        }
    }
  if (!key_handled && keyboard_focus_widget)
    keyboard_focus_widget->key_press_event (event);
  else if (!key_handled)
    {
      if (Debug::enabled ("global")) /* don't do this in production */
        {
          if (event.character == 'g')
            {
              draw_grid = !draw_grid;
              need_update (nullptr, nullptr, UPDATE_MERGE);
            }
          else if (event.character == 'u')
            {
              debug_update_region = !debug_update_region;
              need_update (nullptr, nullptr, UPDATE_MERGE);
            }
        }
    }
}

void
Window::on_close_event (const PuglEventClose& event)
{
  if (m_close_callback)
    m_close_callback();
}

void
Window::on_configure_event (const PuglEventConfigure& event)
{
  int w, h;
  get_scaled_size (&w, &h);
  cairo_gl.reset (new CairoGL (w, h));

  // on windows, the coordinates of the event often doesn't match actual size
  // cairo_gl.reset (new CairoGL (event->configure.width, event->configure.height));

  cairo_gl->configure();
  update_full();
}

Widget *
Window::find_widget_xy (double ex, double ey)
{
  Widget *widget = this;

  if (menu_widget)
    widget = menu_widget;  // active menu => only children of the menu get clicks

  if (dialog_widget)
    widget = dialog_widget;

  for (auto w : ::crawl_widgets ({ widget })) // which child gets the click?
    {
      if (get_visible_recursive (w) && w->recursive_enabled() && w->abs_visible_rect().contains (ex, ey))
        {
          widget = w;
        }
    }
  return widget;
}

void
Window::show()
{
  puglPostRedisplay (view);
  puglShowWindow (view);
}

void
Window::open_file_dialog (const string& title, const FileDialogFormats& formats, std::function<void(string)> callback)
{
  file_dialog_callback = callback;
  have_file_dialog = true;

  native_file_dialog.reset (NativeFileDialog::create (this, true, title, formats));
  connect (native_file_dialog->signal_file_selected, this, &Window::on_file_selected);
  update_full();
}

void
Window::save_file_dialog (const string& title, const FileDialogFormats& formats, std::function<void(string)> callback)
{
  file_dialog_callback = callback;
  have_file_dialog = true;

  native_file_dialog.reset (NativeFileDialog::create (this, false, title, formats));
  connect (native_file_dialog->signal_file_selected, this, &Window::on_file_selected);
  update_full();
}

void
Window::on_file_selected (const std::string& filename)
{
  if (file_dialog_callback)
    {
      file_dialog_callback (filename);
      file_dialog_callback = nullptr;
    }
  have_file_dialog = false;
  update_full();
}

void
Window::need_update (Widget *widget, const Rect *changed_rect, UpdateStrategy update_strategy)
{
  if (widget)
    {
      Rect widget_rect = widget->abs_visible_rect();
      if (changed_rect)
        {
          /* if changed rect is set, we only need to redraw a part of the widget */
          Rect abs_changed_rect;
          abs_changed_rect = Rect (changed_rect->x() + widget->abs_x(),
                                   changed_rect->y() + widget->abs_y(),
                                   changed_rect->width(),
                                   changed_rect->height());
          widget_rect = widget_rect.intersection (abs_changed_rect);
        }
      UpdateRegion region;
      region.widget = widget;
      region.update_strategy = update_strategy;
      region.region = widget_rect;
      update_regions.push_back (region);
    }
  else
    update_full_redraw = true;

  puglPostRedisplay (view);
}

Window *
Window::window()
{
  return this;
}

void
Window::set_menu_widget (Widget *widget)
{
  menu_widget = widget;
  mouse_widget = widget;
}

void
Window::set_close_callback (const std::function<void()>& callback)
{
  m_close_callback = callback;
}

void
Window::set_keyboard_focus (Widget *widget, bool release_on_click)
{
  keyboard_focus_widget = widget;
  keyboard_focus_widget->focus_event();
  keyboard_focus_release_on_click = release_on_click;
}

bool
Window::has_keyboard_focus (Widget *widget)
{
  return keyboard_focus_widget == widget;
}

void
Window::set_dialog_widget (Widget *widget)
{
  dialog_widget = widget;
}

void
Window::set_popup_window (Window *pwin)
{
  if (pwin)
    {
      // take ownership
      popup_window.reset (pwin);
    }
  else
    {
      auto del_window = popup_window.release();

      del_window->delete_later();
    }
  update_full();
}

PuglNativeWindow
Window::native_window()
{
  return puglGetNativeWindow (view);
}

void
Window::fill_zoom_menu (Menu *menu)
{
  menu->clear();

  for (int z = 70; z <= 500; )
    {
      int w = width() * z / 100;
      int h = height() * z / 100;

      string text = string_locale_printf ("%d%%   -   %dx%d", z, w, h);

      if (sm_round_positive (window()->gui_scaling() * 100) == z)
        text += "   -   current zoom";
      MenuItem *item = menu->add_item (text);
      connect (item->signal_clicked, [=]() {
        window()->set_gui_scaling (z / 100.);

        // we need to refill the menu to update the "current zoom" entry
        fill_zoom_menu (menu);
      });

      if (z >= 400)
        z += 50;
      else if (z >= 300)
        z += 25;
      else if (z >= 200)
        z += 20;
      else
        z += 10;
    }
}

void
Window::set_gui_scaling (double s)
{
  global_scale = s;

  /* restart with this gui scaling next time */
  Config cfg;

  cfg.set_zoom (sm_round_positive (s * 100));
  cfg.store();

  /* (1) typically, at this point we notify the host that our window will have a new size */
  signal_update_size();

  /* (2) and we ensure that our window size will be changed via pugl */
  puglPostResize (view);
}

double
Window::gui_scaling()
{
  return global_scale;
}

void
Window::on_resize (int *win_width, int *win_height)
{
  get_scaled_size (win_width, win_height);
}

void
Window::get_scaled_size (int *w, int *h)
{
  *w = width() * global_scale;
  *h = height() * global_scale;
}

void
Window::add_shortcut (Shortcut *shortcut)
{
  shortcuts.push_back (shortcut);
}

void
Window::remove_shortcut (Shortcut *shortcut)
{
  for (auto& s : shortcuts)
    {
      if (s == shortcut)
        s = nullptr;
    }
}

EventLoop *
Window::event_loop() const
{
  return m_event_loop;
}
