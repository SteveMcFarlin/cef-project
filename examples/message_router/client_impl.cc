// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "examples/message_router/client_impl.h"

#include <algorithm>
#include <iostream>

#include "include/wrapper/cef_helpers.h"

#include "examples/shared/client_util.h"
#include "examples/shared/resource_util.h"

namespace message_router {

namespace {

const char kTestMessageName[] = "MessageRouterTest";

// Handle messages in the browser process.
class MessageHandler : public CefMessageRouterBrowserSide::Handler {
 public:
  explicit MessageHandler(const CefString& startup_url)
      : startup_url_(startup_url) {}

  // Called due to cefQuery execution in message_router.html.
  bool OnQuery(CefRefPtr<CefBrowser> browser,
               CefRefPtr<CefFrame> frame,
               int64_t query_id,
               const CefString& request,
               bool persistent,
               CefRefPtr<Callback> callback) override {
    // Only handle messages from the startup URL.

    std::cout << "MessageHandler - Browser OnQuery: " << request.ToString()
              << std::endl;

    const std::string& url = frame->GetURL();
    if (url.find(startup_url_) != 0)
      return false;

    const std::string& message_name = request;
    if (message_name.find(kTestMessageName) == 0) {
      // if (true) {
      // Reverse the string and return.
      std::string result = message_name.substr(sizeof(kTestMessageName));
      std::reverse(result.begin(), result.end());
      std::cout << "Browser OnQuery result: " << result << std::endl;
      callback->Success(result);
      return true;
    }

    return false;
  }

 private:
  const CefString startup_url_;

  DISALLOW_COPY_AND_ASSIGN(MessageHandler);
};

class MyHandler : public CefMessageRouterBrowserSide::Handler {
 public:
  explicit MyHandler(const CefString& startup_url)
      : startup_url_(startup_url) {}

  // Called due to cefQuery execution in message_router.html.
  bool OnQuery(CefRefPtr<CefBrowser> browser,
               CefRefPtr<CefFrame> frame,
               int64_t query_id,
               const CefString& request,
               bool persistent,
               CefRefPtr<Callback> callback) override {
    // Only handle messages from the startup URL.

    std::cout << "MyHandler Browser OnQuery: " << request.ToString()
              << std::endl;

    const std::string& url = frame->GetURL();
    if (url.find(startup_url_) != 0)
      return false;

    const std::string& message_name = request;
    // if (message_name.find(kTestMessageName) == 0) {
    if (true) {
      // Reverse the string and return.

      callback->Success("result");
      return true;
    }

    return true;
  }

 private:
  const CefString startup_url_;

  DISALLOW_COPY_AND_ASSIGN(MyHandler);
};

}  // namespace

Client::Client(const CefString& startup_url)
    : startup_url_(startup_url), browser_ct_(0) {
  my_load_handler_ = new MyCustomLoadHandler();
}

void Client::OnTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title) {
  // Call the default shared implementation.
  shared::OnTitleChange(browser, title);
}

bool Client::HandleMessage(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefProcessMessage> message) {
  std::cout << "Browser Client HandleMessage: " << message->GetName().ToString()
            << std::endl;

  if (message->GetName() == "roomOpen") {
    std::cout << "Handled roomOpen" << std::endl;
    CEF_REQUIRE_UI_THREAD();

    // Create the message object.
    CefRefPtr<CefProcessMessage> msg =
        CefProcessMessage::Create("exposeFunction");
    // Retrieve the argument list object.
    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    // Populate the argument values.
    args->SetString(0, "testFunction");
    args->SetString(1, "JS CODE");
    // Send the process message to the main frame in the render process.
    // Use PID_BROWSER instead when sending a message to the browser process.
    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    frame->SendProcessMessage(PID_RENDERER, msg);

    return true;
  } else if (message->GetName() == "roomClosed") {
    std::cout << "Handled roomClosed" << std::endl;
    return true;
  } else if (message->GetName() == "testFunction") {
    std::cout << "Handled testFunction" << std::endl;
    return true;
  }

  // CefRefPtr<CefListValue> args = message->GetArgumentList();

  // int size = args->GetSize();
  // for (int i = 0; i < size; i++) {
  //   CefValueType type = args->GetType(i);
  //   switch (type) {
  //     case VTYPE_BOOL:
  //       std::cout << "VTYPE_BOOL: " << args->GetBool(i) << std::endl;
  //       break;
  //     case VTYPE_DOUBLE:
  //       std::cout << "VTYPE_DOUBLE: " << args->GetDouble(i) << std::endl;
  //       break;
  //     case VTYPE_INT:
  //       std::cout << "VTYPE_INT: " << args->GetInt(i) << std::endl;
  //       break;
  //     case VTYPE_STRING:
  //       std::cout << "VTYPE_STRING: " << args->GetString(i).ToString()
  //                 << std::endl;
  //       break;
  //     default:
  //       std::cout << "VTYPE_UNKNOWN: " << std::endl;
  //       break;
  //   }
  // }

  return false;
}

bool Client::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefProcessId source_process,
                                      CefRefPtr<CefProcessMessage> message) {
  CEF_REQUIRE_UI_THREAD();

  std::cout << "Browser Client OnProcessMessageReceived: "
            << message->GetName().ToString() << std::endl;

  CefRefPtr<CefListValue> args = message->GetArgumentList();

  int size = args->GetSize();
  for (int i = 0; i < size; i++) {
    CefValueType type = args->GetType(i);
    switch (type) {
      case VTYPE_BOOL:
        std::cout << "VTYPE_BOOL: " << args->GetBool(i) << std::endl;
        break;
      case VTYPE_DOUBLE:
        std::cout << "VTYPE_DOUBLE: " << args->GetDouble(i) << std::endl;
        break;
      case VTYPE_INT:
        std::cout << "VTYPE_INT: " << args->GetInt(i) << std::endl;
        break;
      case VTYPE_STRING:
        std::cout << "VTYPE_STRING: " << args->GetString(i).ToString()
                  << std::endl;
        break;
      default:
        std::cout << "VTYPE_UNKNOWN: " << std::endl;
        break;
    }
  }

  bool handled = false;
  handled = message_router_->OnProcessMessageReceived(browser, frame,
                                                      source_process, message);
  handled |= my_router_->OnProcessMessageReceived(browser, frame,
                                                  source_process, message);
  handled |= HandleMessage(browser, message);

  std::cout << "Browser Client OnProcessMessageReceived handled: " << handled
            << std::endl;
  return handled;
}

void Client::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (!message_router_) {
    // Create the browser-side router for query handling.
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterBrowserSide::Create(config);
    CefMessageRouterConfig my_config;
    my_config.js_query_function = "queryUpstage";
    my_config.js_cancel_function = "cancelQuery";
    my_router_ = CefMessageRouterBrowserSide::Create(my_config);

    // Register handlers with the router.
    message_handler_.reset(new MessageHandler(startup_url_));
    message_router_->AddHandler(message_handler_.get(), false);

    my_handler_.reset(new MyHandler(startup_url_));
    my_router_->AddHandler(my_handler_.get(), false);
  }

  browser_ct_++;

  // Call the default shared implementation.
  shared::OnAfterCreated(browser);
}

bool Client::DoClose(CefRefPtr<CefBrowser> browser) {
  // Call the default shared implementation.
  return shared::DoClose(browser);
}

void Client::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  if (--browser_ct_ == 0) {
    // Free the router when the last browser is closed.
    message_router_->RemoveHandler(message_handler_.get());
    message_handler_.reset();
    message_router_ = nullptr;
  }

  // Call the default shared implementation.
  shared::OnBeforeClose(browser);
}

bool Client::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            bool user_gesture,
                            bool is_redirect) {
  CEF_REQUIRE_UI_THREAD();

  // Create the message object.
  CefRefPtr<CefProcessMessage> msg =
      CefProcessMessage::Create("exposeFunction");

  // Retrieve the argument list object.
  CefRefPtr<CefListValue> args = msg->GetArgumentList();

  // Populate the argument values.
  args->SetString(0, "roomOpen");
  // args->SetInt(0, 10);

  // Send the process message to the main frame in the render process.
  // Use PID_BROWSER instead when sending a message to the browser process.
  //
  std::cout << "Browser Send Message: " << args->GetString(0).ToString()
            << std::endl;
  browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, msg);

  CefRefPtr<CefProcessMessage> msg2 =
      CefProcessMessage::Create("exposeFunction");
  CefRefPtr<CefListValue> args2 = msg2->GetArgumentList();
  args2->SetString(0, "roomClosed");
  std::cout << "Browser Send Message: " << args2->GetString(0).ToString()
            << std::endl;
  browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, msg2);

  message_router_->OnBeforeBrowse(browser, frame);
  my_router_->OnBeforeBrowse(browser, frame);
  return false;
}

CefRefPtr<CefResourceRequestHandler> Client::GetResourceRequestHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool is_navigation,
    bool is_download,
    const CefString& request_initiator,
    bool& disable_default_handling) {
  CEF_REQUIRE_IO_THREAD();
  return this;
}

void Client::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                       TerminationStatus status) {
  CEF_REQUIRE_UI_THREAD();

  message_router_->OnRenderProcessTerminated(browser);
  my_router_->OnRenderProcessTerminated(browser);
}

CefRefPtr<CefResourceHandler> Client::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request) {
  CEF_REQUIRE_IO_THREAD();

  const std::string& url = request->GetURL();

  // This is a minimal implementation of resource loading. For more complex
  // usage (multiple files, zip archives, custom handlers, etc.) you might
  // want to use CefResourceManager. See the "resource_manager" target for an
  // example implementation.
  const std::string& resource_path = shared::GetResourcePath(url);
  if (!resource_path.empty())
    return shared::GetResourceHandler(resource_path);

  return nullptr;
}

}  // namespace message_router
