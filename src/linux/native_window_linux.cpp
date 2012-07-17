#include <node.h>
#include <gtk/gtk.h>
#include "appjs.h"
#include "base/native_window.h"
#include "includes/cef.h"
#include "includes/util.h"
#include "includes/cef_handler.h"

extern CefRefPtr<ClientHandler> g_handler;

namespace appjs {

using namespace v8;

void destroy_handler(GtkWidget* widget, NativeWindow* window) {
  Handle<Object> handle = window->GetV8Handle();
  Handle<Value> argv[1] = {String::New("close")};
  node::MakeCallback(handle,"emit",1,argv);
}

void drag_handler( GtkWidget* widget, GdkEvent* event, NativeWindow* window ) {
  gtk_window_begin_move_drag(GTK_WINDOW(widget), event->type, event->button.x_root,event->button.y_root,event->button.time);
  g_signal_handler_disconnect(G_OBJECT(widget), window->GetDragHandlerId());
}

void NativeWindow::Init(char* url, Settings* settings) {
  handle_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWindow* window = (GtkWindow*)handle_;

  // Set default icon list
  if( !g_handler->GetBrowserHwnd() ) {

    char* smallerIconPath = icons->getString("smaller","");
    char* smallIconPath = icons->getString("small","");
    char* bigIconPath = icons->getString("big","");
    char* biggerIconPath = icons->getString("bigger","");

    GdkPixbuf* smallerIconBuf = gdk_pixbuf_new_from_file(smallerIconPath,NULL);
    GdkPixbuf* smallIconBuf = gdk_pixbuf_new_from_file(smallIconPath,NULL);
    GdkPixbuf* bigIconBuf = gdk_pixbuf_new_from_file(bigIconPath,NULL);
    GdkPixbuf* biggerIconBuf = gdk_pixbuf_new_from_file(biggerIconPath,NULL);

    GList* iconList;

    iconList = g_list_insert(iconList,smallerIconBuf,1);
    iconList = g_list_insert(iconList,smallIconBuf,0);
    iconList = g_list_insert(iconList,bigIconBuf,2);
    iconList = g_list_insert(iconList,biggerIconBuf,3);

    gtk_window_set_default_icon_list(iconList);

//    delete smallerIconPath;
//    delete smallIconPath;
//    delete bigIconPath;
//    delete biggerIconPath;
  }

  gtk_window_set_default_size(window, width_, height_);
  gtk_window_set_opacity(window, opacity);
  gtk_window_set_decorated(window, show_chrome);

  #if defined(__UBUNTU__) && !GTK_CHECK_VERSION(2, 24, 10)
    if(gtk_check_version(2, 24, 10))
      gtk_window_set_has_resize_grip(window, show_resize_grip);
  #endif

  if( fullscreen ) {
    gtk_window_fullscreen(window);
  } else {
    gtk_window_set_resizable(window, resizable);
  }

  if( !resizable ) {
    gtk_widget_set_size_request(handle_,width_,height_);
  }

  if( left_ == -1 ) {
    left_ = (ScreenWidth() - width_) / 2;
  }

  if ( top_ == -1 ) {
    top_ = (ScreenHeight() - height_) / 2;
  }

  gtk_window_move(window,left_,top_);

  GtkWidget* box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(handle_), box);

  g_signal_connect(G_OBJECT(handle_), "destroy",
                   G_CALLBACK(destroy_handler), this);

  g_object_set_data(G_OBJECT(handle_),"nativewindow",this);

  gtk_widget_add_events(handle_,  GDK_POINTER_MOTION_MASK |
                                  GDK_BUTTON_PRESS_MASK |
                                  GDK_BUTTON_RELEASE_MASK);
  Cef::AddWebView(box,url,settings);
}

long NativeWindow::GetDragHandlerId() {
  return drag_handler_id;
}

int NativeWindow::ScreenWidth() {
  GdkScreen* screen = gdk_screen_get_default();
  return gdk_screen_get_width(screen);
}

int NativeWindow::ScreenHeight() {
  GdkScreen* screen = gdk_screen_get_default();
  return gdk_screen_get_height(screen);
}

void NativeWindow::Minimize() {
  gtk_window_iconify((GtkWindow*)handle_);
}

void NativeWindow::Maximize() {
  gtk_window_maximize((GtkWindow*)handle_);
}

void NativeWindow::Restore() {
  gtk_window_deiconify((GtkWindow*)handle_);
}

void NativeWindow::Show() {
  gtk_widget_show_all(handle_);
}

void NativeWindow::Hide() {
  gtk_widget_hide(handle_);
}

void NativeWindow::Destroy() {
  gtk_widget_destroy(handle_);
}

void NativeWindow::Drag() {
  // TODO we need to emit mouse button press manually
  // after connecting the signal below. the problem is
  // about emitting that event.
  //drag_handler_id = g_signal_connect(G_OBJECT(handle_),"button-press-event",G_CALLBACK(drag_handler), this);
  /*GdkEvent* event = gdk_event_new (GDK_BUTTON_PRESS);
  event->button.window = handle_->window;
  event->button.send_event = TRUE;
  event->button.x_root = 10;
  event->button.y_root = 10;
  event->button.x = 10;
  event->button.y = 10;
  event->button.time = GDK_CURRENT_TIME;
  event->button.button = 1;*/

}

void NativeWindow::Drop() {
}

void NativeWindow::SetPosition(int top, int left, int width, int height) {
  GtkWindow* window = (GtkWindow*)handle_;
  gtk_window_move(window,top,left);
  gtk_window_resize(window,width,height);
}

void NativeWindow::SetPosition(int top, int left) {
  top_ = top;
  left_ = left;
  gtk_window_move((GtkWindow*)handle_,top,left);
}

void NativeWindow::SetSize(int width, int height) {
  width_ = width;
  height_ = height;
  gtk_window_resize((GtkWindow*)handle_,width,height);
}

const char* NativeWindow::GetTitle() {
  return gtk_window_get_title((GtkWindow*)handle_);
}


} /* appjs */
