// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <iostream>
#include "examples/shared/app_factory.h"

#include "include/wrapper/cef_message_router.h"

namespace message_router {
namespace {

const char kTestMessageName[] = "MessageRouterTest";

class MyV8Handler : public CefV8Handler {
 public:
  MyV8Handler() {}

  void SendProcessMessage(std::string message) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(message);
    std::cout << "Render SendProcessMessage: " << message << std::endl;
    // CefRefPtr<CefListValue> args = msg->GetArgumentList();
    // args->SetString(0, "string_arg");
    browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) override {
    std::cout << "Render Execute: " << name.ToString() << std::endl;
    if (name == "cefVersion") {
      retval = CefV8Value::CreateString("CEF v0.1.0");
      return true;
    } else if (name == "roomOpen") {
      std::cout << "Render:roomOpen - Render Value!" << std::endl;
      retval = CefV8Value::CreateString("roomOpen called");
      SendProcessMessage(name);

      return true;
    } else if (name == "roomClosed") {
      std::cout << "Render:roomClosed - Render Value!" << std::endl;
      retval = CefV8Value::CreateString("roomClosed called");
      SendProcessMessage(name);
      return true;
    } else if (name == "testFunction") {
      std::cout << "Render:testFunction!" << std::endl;
      retval = CefV8Value::CreateString("Render testFunction!");
      SendProcessMessage(name);
      return true;
    }

    // Function does not exist.
    return false;
  }

  CefRefPtr<CefBrowser> browser;
  // Provide the reference counting implementation for this class.
  IMPLEMENT_REFCOUNTING(MyV8Handler);
};

// Implementation of CefApp for the renderer process.
class RendererApp : public CefApp, public CefRenderProcessHandler {
 public:
  RendererApp() {}

  // CefApp methods:
  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
    return this;
  }

  // CefRenderProcessHandler methods:
  void OnWebKitInitialized() override {
    // Create the renderer-side router for query handling.
    // For now just use the default cefQuery. We are not using that
    // functionality at the moment
    CefMessageRouterConfig config;
    config.js_query_function = "queryUpstage";
    config.js_cancel_function = "cancelQuery";
    message_router_ = CefMessageRouterRendererSide::Create(config);
    handler_ = new MyV8Handler();
    // message_handler_.reset(new MessageHandler("startup_url_ "));
  }

  void OnContextCreated(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) override {
    message_router_->OnContextCreated(browser, frame, context);
    // Register function handlers with the 'window' object.

    CefRefPtr<CefV8Value> window = context->GetGlobal();
    handler_->browser = browser;
    // Create the query function.
    CefRefPtr<CefV8Value> func =
        CefV8Value::CreateFunction("cefVersion", handler_);
    window->SetValue("cefVersion", func, V8_PROPERTY_ATTRIBUTE_NONE);
    std::cout << "Render - Added function to window:  "
              << func->GetFunctionName() << std::endl;

    // CefRefPtr<CefV8Value> window = context->GetGlobal();
    // CefRefPtr<CefV8Handler> handler = new MyV8Handler();
    // Create the query function.

    // CefRefPtr<CefV8Value> room_open =
    //     CefV8Value::CreateFunction("roomOpen", handler_);
    // window->SetValue("roomOpen", room_open, V8_PROPERTY_ATTRIBUTE_NONE);
    // std::cout << "Render - Added function to window:  "
    //           << room_open->GetFunctionName() << std::endl;

    // CefRefPtr<CefV8Value> room_closed =
    //     CefV8Value::CreateFunction("roomClosed", handler_);
    // window->SetValue("roomClosed", room_closed, V8_PROPERTY_ATTRIBUTE_NONE);
    // std::cout << "Render - Added function to window:  "
    //           << room_closed->GetFunctionName() << std::endl;

    // CefRefPtr<CefV8Context> context = frame->GetV8Context();
    // CefRefPtr<CefV8Value> window = context->GetGlobal();
    // CefRefPtr<CefV8Value> new_fn =
    //     CefV8Value::CreateFunction("testFunction", handler_);
    // window->SetValue("testFunction", new_fn, V8_PROPERTY_ATTRIBUTE_NONE);

    // CefMessageRouterConfig config;
    // CefRefPtr<V8HandlerImpl> handler = new V8HandlerImpl(this, config);
    // CefV8Value::PropertyAttribute attributes =
    //     static_cast<CefV8Value::PropertyAttribute>(
    //         V8_PROPERTY_ATTRIBUTE_READONLY | V8_PROPERTY_ATTRIBUTE_DONTENUM |
    //         V8_PROPERTY_ATTRIBUTE_DONTDELETE);

    // // Add the query function.
    // CefRefPtr<CefV8Value> query_func =
    //     CefV8Value::CreateFunction(config_.js_query_function, handler.get());
    // window->SetValue(config_.js_query_function, query_func, attributes);

    // // Add the cancel function.
    // CefRefPtr<CefV8Value> cancel_func =
    //     CefV8Value::CreateFunction(config_.js_cancel_function,
    //     handler.get());
    // window->SetValue(config_.js_cancel_function, cancel_func, attributes);
  }

  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override {
    message_router_->OnContextReleased(browser, frame, context);
  }

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override {
    bool handled = false;

    std::cout << "Render OnProcessMessageReceived: "
              << message->GetName().ToString() << std::endl;
    CefRefPtr<CefListValue> args = message->GetArgumentList();
    int size = args->GetSize();
    std::cout << "args size: " << size << std::endl;

    for (int i = 0; i < size; i++) {
      CefValueType type = args->GetType(i);
      std::cout << "args[" << i << "] type: " << type << std::endl;

      if (type == VTYPE_STRING) {
        std::cout << "args[ " << i << "]: " << args->GetString(i).ToString()
                  << std::endl;
      }
    }

    std::string name = message->GetName().ToString();
    if (name == "exposeFunction") {
      std::string fn_name = args->GetString(0).ToString();
      std::cout << "Render exposeFunction: " << fn_name << std::endl;
      CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
      context->Enter();

      CefRefPtr<CefV8Value> window = context->GetGlobal();

      CefRefPtr<CefV8Value> new_fn =
          CefV8Value::CreateFunction(fn_name, handler_);
      window->SetValue(fn_name, new_fn, V8_PROPERTY_ATTRIBUTE_NONE);
      std::cout << "Added function to window: " << new_fn->GetFunctionName()
                << std::endl;

      // Test calling the function
      CefRefPtr<CefV8Value> function = window->GetValue("showAlert");

      if (function.get() && function->IsFunction()) {
        CefV8ValueList args;
        CefRefPtr<CefV8Value> retval = function->ExecuteFunction(nullptr, args);
        std::cout << "Function return value null?: " << retval->IsNull()
                  << std::endl;
        // Handle the return value or exceptions if needed
      } else {
        std::cout << "Function does not exist!" << std::endl;
      }

      context->Exit();
      handled = true;
    } else {
      handled = message_router_->OnProcessMessageReceived(
          browser, frame, source_process, message);
    }

    // std::cout << "\n\nRender OnProcessMessageReceived executing JS: "
    //           << message->GetName().ToString() << std::endl;
    // frame->ExecuteJavaScript("alert(\" ExecuteJS \");", frame->GetURL(), 0);

    std::cout << "Render OnProcessMessageReceived handled: " << handled
              << std::endl
              << std::endl;
    return handled;
  }

 private:
  // Handles the renderer side of query routing.
  CefRefPtr<CefMessageRouterRendererSide> message_router_;
  CefRefPtr<MyV8Handler> handler_;
  // std::unique_ptr<CefMessageRouterBrowserSide::Handler> message_handler_;

  IMPLEMENT_REFCOUNTING(RendererApp);
  DISALLOW_COPY_AND_ASSIGN(RendererApp);
};
}  // namespace
}  // namespace message_router

namespace shared {

CefRefPtr<CefApp> CreateRendererProcessApp() {
  return new message_router::RendererApp();
}

}  // namespace shared
