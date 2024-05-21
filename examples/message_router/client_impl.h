// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_EXAMPLES_MESSAGE_ROUTER_CLIENT_IMPL_H_
#define CEF_EXAMPLES_MESSAGE_ROUTER_CLIENT_IMPL_H_

#include <iostream>
#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

namespace message_router {

class MyCustomLoadHandler : public CefLoadHandler {
 public:
  // Called when a frame starts loading.
  virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           TransitionType transition_type) override {
    // This method is called when a frame starts loading in the browser.
    if (frame->IsMain()) {
      // The main frame has started loading.
      std::cout << "Main frame starts loading." << std::endl;
    } else {
      // A sub-frame has started loading.
      std::cout << "Sub-frame starts loading. URL: "
                << frame->GetURL().ToString() << std::endl;
    }
  }

  // Called when a frame finishes loading.
  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode) override {
    // This method is called when a frame finishes loading in the browser.
    if (frame->IsMain()) {
      // The main frame has finished loading.
      std::cout << "Main frame finished loading. Status code: "
                << httpStatusCode << std::endl;
    } else {
      // A sub-frame has finished loading.
      std::cout << "Sub-frame finished loading. URL: "
                << frame->GetURL().ToString()
                << " Status code: " << httpStatusCode << std::endl;
    }
  }

  IMPLEMENT_REFCOUNTING(MyCustomLoadHandler);
};

// Implementation of client handlers.
class Client : public CefClient,
               public CefDisplayHandler,
               public CefLifeSpanHandler,
               public CefRequestHandler,
               public CefResourceRequestHandler,
               public CefLoadHandler {
 public:
  explicit Client(const CefString& startup_url);

  // CefClient methods:
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override;

  // CefDisplayHandler methods:
  void OnTitleChange(CefRefPtr<CefBrowser> browser,
                     const CefString& title) override;

  virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                    bool isLoading,
                                    bool canGoBack,
                                    bool canGoForward) {
    std::cout << "### OnLoadingStateChange" << std::endl;
  }
  virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame) {
    std::cout << "### OnLoadStart" << std::endl;
  }
  virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int httpStatusCode) {
    std::cout << "### OnLoadEnd" << std::endl;
  }
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) {
    std::cout << "### OnLoadError" << std::endl;
  }

  // CefLifeSpanHandler methods:
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefRequestHandler methods:
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request,
                      bool user_gesture,
                      bool is_redirect) override;
  CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request,
      bool is_navigation,
      bool is_download,
      const CefString& request_initiator,
      bool& disable_default_handling) override;
  void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                 TerminationStatus status) override;

  // CefResourceRequestHandler methods:
  CefRefPtr<CefResourceHandler> GetResourceHandler(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request) override;

 private:
  bool HandleMessage(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefProcessMessage> message);

 private:
  // Handles the browser side of query routing.
  CefRefPtr<CefMessageRouterBrowserSide> message_router_;
  std::unique_ptr<CefMessageRouterBrowserSide::Handler> message_handler_;
  CefRefPtr<CefMessageRouterBrowserSide> my_router_;
  std::unique_ptr<CefMessageRouterBrowserSide::Handler> my_handler_;
  CefRefPtr<MyCustomLoadHandler> my_load_handler_;
  const CefString startup_url_;

  // Track the number of browsers using this Client.
  int browser_ct_;

  IMPLEMENT_REFCOUNTING(Client);
  DISALLOW_COPY_AND_ASSIGN(Client);
};

}  // namespace message_router

#endif  // CEF_EXAMPLES_MESSAGE_ROUTER_CLIENT_IMPL_H_
